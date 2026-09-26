package com.smarthive.ble

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.smarthive.ble.ble.BleScanner
import com.smarthive.ble.ble.ConnectionState
import com.smarthive.ble.ble.HiveField
import com.smarthive.ble.ble.HiveProgram
import com.smarthive.ble.ble.SmartHiveConnection
import com.smarthive.ble.ble.SmartHiveDevice
import com.smarthive.ble.ble.SmartHiveProtocol
import com.smarthive.ble.data.AuthRepository
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlin.math.sin
import kotlin.random.Random

data class Sample(val timestampMs: Long, val temperature: Float, val humidity: Float?)

class MainViewModel(application: Application) : AndroidViewModel(application) {

    private val auth = AuthRepository(application)
    private val scanner = BleScanner(application)
    private val connection = SmartHiveConnection(application)

    private val _loggedInUser = MutableStateFlow<String?>(null)
    val loggedInUser: StateFlow<String?> = _loggedInUser.asStateFlow()

    private val _devices = MutableStateFlow<List<SmartHiveDevice>>(emptyList())
    val devices: StateFlow<List<SmartHiveDevice>> = _devices.asStateFlow()

    private val _scanning = MutableStateFlow(false)
    val scanning: StateFlow<Boolean> = _scanning.asStateFlow()

    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()

    private val _selectedDevice = MutableStateFlow<SmartHiveDevice?>(null)
    val selectedDevice: StateFlow<SmartHiveDevice?> = _selectedDevice.asStateFlow()

    private val _history = MutableStateFlow<List<Sample>>(emptyList())
    val history: StateFlow<List<Sample>> = _history.asStateFlow()

    private val _demoMode = MutableStateFlow(false)
    private val _demoState = MutableStateFlow(ConnectionState.Disconnected)
    private val _demoValues = MutableStateFlow<Map<HiveField, String>>(emptyMap())

    val connectionState: StateFlow<ConnectionState> =
        combine(_demoMode, _demoState, connection.state) { demo, demoState, liveState ->
            if (demo) demoState else liveState
        }.stateIn(viewModelScope, SharingStarted.Eagerly, ConnectionState.Disconnected)

    val values: StateFlow<Map<HiveField, String>> =
        combine(_demoMode, _demoValues, connection.values) { demo, demoValues, liveValues ->
            if (demo) demoValues else liveValues
        }.stateIn(viewModelScope, SharingStarted.Eagerly, emptyMap())

    val autoProgram: StateFlow<HiveProgram?> = values
        .map { SmartHiveProtocol.parseProgram(it[HiveField.AutoProgram]) }
        .stateIn(viewModelScope, SharingStarted.Eagerly, null)

    val log: StateFlow<List<String>> = connection.log

    private var scanJob: Job? = null
    private var demoJob: Job? = null
    private var pollJob: Job? = null

    val defaultCredentialsHint =
        "${AuthRepository.DEFAULT_USER} / ${AuthRepository.DEFAULT_PASSWORD}"

    fun login(username: String, password: String): Boolean {
        val ok = auth.login(username, password)
        if (ok) _loggedInUser.value = username.trim().lowercase()
        else _error.value = "Invalid username or password"
        return ok
    }

    fun register(username: String, password: String): Boolean {
        val ok = auth.register(username, password)
        if (!ok) _error.value = "Could not create that account (already exists or password too short)"
        return ok
    }

    fun logout() {
        stopScan()
        disconnect()
        _loggedInUser.value = null
    }

    fun startScan() {
        if (_scanning.value) return
        if (!scanner.isBluetoothEnabled) {
            _error.value = "Bluetooth is off. Enable it and scan again."
            return
        }
        _error.value = null
        _devices.value = emptyList()
        _scanning.value = true
        scanJob = viewModelScope.launch {
            scanner.scan()
                .catch { throwable ->
                    _error.value = throwable.message
                    _scanning.value = false
                }
                .collect { _devices.value = it }
        }
    }

    fun stopScan() {
        scanJob?.cancel()
        scanJob = null
        _scanning.value = false
    }

    fun connect(device: SmartHiveDevice) {
        stopScan()
        _selectedDevice.value = device
        _history.value = emptyList()
        _demoMode.value = device.address == DEMO_ADDRESS
        if (_demoMode.value) startDemo() else connection.connect(device.address)
        startPolling()
        trackHistory()
    }

    fun disconnect() {
        demoJob?.cancel()
        pollJob?.cancel()
        historyJob?.cancel()
        connection.disconnect()
        _demoState.value = ConnectionState.Disconnected
        _demoValues.value = emptyMap()
        _demoMode.value = false
        _selectedDevice.value = null
    }

    fun refresh() {
        if (!_demoMode.value) connection.refresh()
    }

    fun write(field: HiveField, text: String) {
        if (_demoMode.value) _demoValues.value = _demoValues.value + (field to text)
        else connection.write(field, text)
    }

    fun writeAutoProgram(program: HiveProgram) {
        write(HiveField.AutoProgram, SmartHiveProtocol.formatProgram(program))
    }

    fun clearError() {
        _error.value = null
    }

    fun addDemoDevice() {
        if (_devices.value.any { it.address == DEMO_ADDRESS }) return
        _devices.value = _devices.value + SmartHiveDevice(
            address = DEMO_ADDRESS,
            name = "SmartHive0000",
            id = "0000",
            rssi = -40,
        )
    }

    private var historyJob: Job? = null

    private fun startPolling() {
        pollJob?.cancel()
        if (_demoMode.value) return
        pollJob = viewModelScope.launch {
            while (isActive) {
                delay(POLL_INTERVAL_MS)
                if (connection.state.value == ConnectionState.Connected) connection.refresh()
            }
        }
    }

    private fun trackHistory() {
        historyJob?.cancel()
        historyJob = viewModelScope.launch {
            var lastSampleAt = 0L
            var lastTemperature: Float? = null
            while (isActive) {
                val current = values.value
                val temperature = SmartHiveProtocol.numberOrNull(current[HiveField.TempInside])
                val humidity = SmartHiveProtocol.numberOrNull(current[HiveField.HumInside])
                val now = System.currentTimeMillis()
                if (temperature != null &&
                    (temperature != lastTemperature || now - lastSampleAt >= MIN_SAMPLE_GAP_MS)
                ) {
                    lastTemperature = temperature
                    lastSampleAt = now
                    _history.value =
                        (_history.value + Sample(now, temperature, humidity)).takeLast(MAX_SAMPLES)
                }
                delay(500)
            }
        }
    }

    private fun startDemo() {
        _demoState.value = ConnectionState.Connected
        demoJob?.cancel()
        demoJob = viewModelScope.launch {
            _demoValues.value = mapOf(
                HiveField.SerialNumber to "SH-DEMO-0000",
                HiveField.SoftwareVersion to "1.0.0-demo",
                HiveField.HardwareVersion to "rev.C",
                HiveField.AutoProgram to SmartHiveProtocol.formatProgram(HiveProgram.Default),
                HiveField.DeviceName to "SmartHive0000",
                HiveField.Position to "35.715300,51.404300",
                HiveField.Time to SmartHiveProtocol.now(),
                HiveField.Date to SmartHiveProtocol.today(),
                HiveField.PidCoeff to "2.00,0.50,0.10",
                HiveField.HiveState to "AUTO",
                HiveField.HeaterState to "ON",
                HiveField.FanState to "OFF",
            )
            var tick = 0
            while (isActive) {
                _demoValues.value = _demoValues.value + mapOf(
                    HiveField.TempInside to "%.1f".format(34f + 1.5f * sin(tick / 6f) + Random.nextFloat() / 2),
                    HiveField.HumInside to "%.1f".format(58f + 4f * sin(tick / 9f) + Random.nextFloat()),
                    HiveField.TempOutside to "%.1f".format(21f + 2f * sin(tick / 11f)),
                    HiveField.HumOutside to "%.1f".format(44f + 3f * sin(tick / 7f)),
                    HiveField.HeaterCurrent to "%.2f".format(1.2f + Random.nextFloat() / 5),
                )
                tick++
                delay(1000)
            }
        }
    }

    override fun onCleared() {
        connection.disconnect()
        super.onCleared()
    }

    companion object {
        const val DEMO_ADDRESS = "DE:M0:00:00:00:00"
        private const val MAX_SAMPLES = 720
        private const val MIN_SAMPLE_GAP_MS = 10_000L
        private const val POLL_INTERVAL_MS = 3_000L
    }
}

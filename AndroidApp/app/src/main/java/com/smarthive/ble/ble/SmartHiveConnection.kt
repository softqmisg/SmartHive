package com.smarthive.ble.ble

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.os.Build
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import java.util.ArrayDeque

enum class ConnectionState { Disconnected, Connecting, Connected, Failed }

/**
 * GATT client for one SmartHive board.
 *
 * All characteristics carry UTF-8 text. Android allows a single GATT operation in flight, so
 * reads, writes and CCCD writes go through [operations] and the next one starts from the
 * completion callback of the previous one. Characteristics that support notifications are
 * subscribed to; the rest are refreshed by [refresh] on the caller's polling schedule.
 */
class SmartHiveConnection(private val context: Context) {

    private val _state = MutableStateFlow(ConnectionState.Disconnected)
    val state: StateFlow<ConnectionState> = _state.asStateFlow()

    private val _values = MutableStateFlow<Map<HiveField, String>>(emptyMap())
    val values: StateFlow<Map<HiveField, String>> = _values.asStateFlow()

    private val _log = MutableStateFlow<List<String>>(emptyList())
    val log: StateFlow<List<String>> = _log.asStateFlow()

    private var gatt: BluetoothGatt? = null
    private val characteristics = mutableMapOf<HiveField, BluetoothGattCharacteristic>()

    private sealed interface Operation {
        data class Read(val field: HiveField) : Operation
        data class Write(val field: HiveField, val text: String) : Operation
        data class Subscribe(val field: HiveField) : Operation
    }

    private val operations = ArrayDeque<Operation>()
    private var busy = false

    val availableFields: Set<HiveField> get() = characteristics.keys.toSet()

    @SuppressLint("MissingPermission")
    fun connect(address: String) {
        disconnect()
        val manager = context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
        val device = manager?.adapter?.getRemoteDevice(address)
        if (device == null) {
            _state.value = ConnectionState.Failed
            return
        }
        _state.value = ConnectionState.Connecting
        gatt = device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE)
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        synchronized(this) {
            operations.clear()
            busy = false
            characteristics.clear()
        }
        gatt?.let {
            it.disconnect()
            it.close()
        }
        gatt = null
        _values.value = emptyMap()
        _state.value = ConnectionState.Disconnected
    }

    /** Re-reads the live values plus anything else the board does not notify about. */
    fun refresh(fields: Collection<HiveField> = HiveField.polled) {
        fields.filter { characteristics.containsKey(it) }.forEach { enqueue(Operation.Read(it)) }
    }

    fun write(field: HiveField, text: String) {
        if (!characteristics.containsKey(field)) {
            appendLog("! ${field.label} is not exposed by this board")
            return
        }
        enqueue(Operation.Write(field, text))
        enqueue(Operation.Read(field))
    }

    @Synchronized
    private fun enqueue(operation: Operation) {
        operations.addLast(operation)
        runNext()
    }

    @Synchronized
    private fun completeOperation() {
        busy = false
        runNext()
    }

    @SuppressLint("MissingPermission")
    @Synchronized
    private fun runNext() {
        if (busy) return
        val activeGatt = gatt ?: return
        val operation = operations.pollFirst() ?: return
        busy = true
        val started = when (operation) {
            is Operation.Read -> characteristics[operation.field]?.let(activeGatt::readCharacteristic)
            is Operation.Write -> writeCharacteristic(activeGatt, operation)
            is Operation.Subscribe -> subscribe(activeGatt, operation.field)
        } ?: false
        if (!started) {
            busy = false
            if (operation !is Operation.Subscribe) appendLog("! could not start $operation")
            runNext()
        }
    }

    @SuppressLint("MissingPermission")
    private fun writeCharacteristic(gatt: BluetoothGatt, operation: Operation.Write): Boolean {
        val characteristic = characteristics[operation.field] ?: return false
        val payload = SmartHiveProtocol.encode(operation.text)
        val writeType =
            if (characteristic.properties and BluetoothGattCharacteristic.PROPERTY_WRITE != 0) {
                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
            } else {
                BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            }
        appendLog("→ ${operation.field.label} = ${operation.text}")
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeCharacteristic(characteristic, payload, writeType) == BluetoothGatt.GATT_SUCCESS
        } else {
            @Suppress("DEPRECATION")
            run {
                characteristic.writeType = writeType
                characteristic.value = payload
                gatt.writeCharacteristic(characteristic)
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun subscribe(gatt: BluetoothGatt, field: HiveField): Boolean {
        val characteristic = characteristics[field] ?: return false
        gatt.setCharacteristicNotification(characteristic, true)
        // The board notifies unconditionally and exposes no CCCD, so local registration is all
        // that is needed there; boards that do expose one still get it written.
        val descriptor = characteristic.getDescriptor(SmartHiveProtocol.CCCD) ?: return false
        val value =
            if (characteristic.properties and BluetoothGattCharacteristic.PROPERTY_NOTIFY != 0) {
                BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            } else {
                BluetoothGattDescriptor.ENABLE_INDICATION_VALUE
            }
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeDescriptor(descriptor, value) == BluetoothGatt.GATT_SUCCESS
        } else {
            @Suppress("DEPRECATION")
            run {
                descriptor.value = value
                gatt.writeDescriptor(descriptor)
            }
        }
    }

    private fun appendLog(line: String) {
        _log.value = (_log.value + line).takeLast(100)
    }

    private fun publish(field: HiveField, text: String) {
        _values.value = _values.value + (field to text)
    }

    private val callback = object : BluetoothGattCallback() {

        @SuppressLint("MissingPermission")
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> gatt.discoverServices()

                BluetoothProfile.STATE_DISCONNECTED -> {
                    synchronized(this@SmartHiveConnection) {
                        operations.clear()
                        busy = false
                    }
                    _state.value =
                        if (status == BluetoothGatt.GATT_SUCCESS) ConnectionState.Disconnected
                        else ConnectionState.Failed
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                _state.value = ConnectionState.Failed
                return
            }
            characteristics.clear()
            gatt.services.flatMap { it.characteristics }.forEach { characteristic ->
                HiveField.byUuid[characteristic.uuid]?.let { characteristics[it] = characteristic }
            }
            appendLog("discovered ${characteristics.size} known characteristics")
            _state.value =
                if (characteristics.isEmpty()) ConnectionState.Failed else ConnectionState.Connected

            characteristics.forEach { (field, characteristic) ->
                if (characteristic.supportsNotify()) enqueue(Operation.Subscribe(field))
                if (characteristic.readable()) enqueue(Operation.Read(field))
            }
        }

        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray,
            status: Int,
        ) {
            handleValue(characteristic, value, status)
            completeOperation()
        }

        @Deprecated("Deprecated in Java")
        @Suppress("DEPRECATION")
        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int,
        ) {
            handleValue(characteristic, characteristic.value ?: ByteArray(0), status)
            completeOperation()
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int,
        ) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                appendLog("! write failed on ${characteristic.uuid} (status $status)")
            }
            completeOperation()
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt,
            descriptor: BluetoothGattDescriptor,
            status: Int,
        ) = completeOperation()

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray,
        ) = handleValue(characteristic, value, BluetoothGatt.GATT_SUCCESS)

        @Deprecated("Deprecated in Java")
        @Suppress("DEPRECATION")
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
        ) {
            handleValue(characteristic, characteristic.value ?: ByteArray(0), BluetoothGatt.GATT_SUCCESS)
        }
    }

    private fun handleValue(
        characteristic: BluetoothGattCharacteristic,
        value: ByteArray,
        status: Int,
    ) {
        if (status != BluetoothGatt.GATT_SUCCESS) return
        val field = HiveField.byUuid[characteristic.uuid] ?: return
        val text = SmartHiveProtocol.decode(value)
        if (text.isEmpty()) return
        appendLog("← ${field.label} = $text")
        publish(field, text)
    }

    private fun BluetoothGattCharacteristic.supportsNotify() =
        properties and (BluetoothGattCharacteristic.PROPERTY_NOTIFY or
            BluetoothGattCharacteristic.PROPERTY_INDICATE) != 0

    private fun BluetoothGattCharacteristic.readable() =
        properties and BluetoothGattCharacteristic.PROPERTY_READ != 0
}

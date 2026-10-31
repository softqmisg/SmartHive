package com.smarthive.ble.ble

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow

data class SmartHiveDevice(
    val address: String,
    val name: String,
    val id: String,
    val rssi: Int,
)

/** Scans for advertising peripherals whose name matches `SmartHive<number>`. */
class BleScanner(private val context: Context) {

    private val adapter: BluetoothAdapter?
        get() = (context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager)?.adapter

    val isBluetoothEnabled: Boolean
        get() = adapter?.isEnabled == true

    @SuppressLint("MissingPermission")
    fun scan(): Flow<List<SmartHiveDevice>> = callbackFlow {
        val scanner = adapter?.bluetoothLeScanner
        if (scanner == null) {
            close(IllegalStateException("Bluetooth is not available"))
            return@callbackFlow
        }

        val found = linkedMapOf<String, SmartHiveDevice>()

        val callback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                val name = result.device.name ?: result.scanRecord?.deviceName ?: return
                val id = SmartHiveProtocol.deviceIdOrNull(name) ?: return
                found[result.device.address] = SmartHiveDevice(
                    address = result.device.address,
                    name = name,
                    id = id,
                    rssi = result.rssi,
                )
                trySend(found.values.sortedByDescending { it.rssi })
            }

            override fun onScanFailed(errorCode: Int) {
                close(IllegalStateException("BLE scan failed with error $errorCode"))
            }
        }

        val settings = ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
            .build()

        scanner.startScan(emptyList(), settings, callback)
        trySend(emptyList())
        awaitClose { runCatching { scanner.stopScan(callback) } }
    }
}

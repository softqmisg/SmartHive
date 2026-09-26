package com.smarthive.ble

import com.smarthive.ble.ble.HiveField
import com.smarthive.ble.ble.SmartHiveProtocol
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class SmartHiveProtocolTest {

    @Test
    fun `matches only SmartHive names with a four character id`() {
        assertEquals("0A17", SmartHiveProtocol.deviceIdOrNull("SmartHive0A17"))
        assertNull(SmartHiveProtocol.deviceIdOrNull("SmartHive017"))
        assertNull(SmartHiveProtocol.deviceIdOrNull("OtherHive0A17"))
    }

    @Test
    fun `extracts numbers from text values`() {
        assertEquals(24.6f, SmartHiveProtocol.numberOrNull("24.6")!!, 0.001f)
        assertEquals(24.6f, SmartHiveProtocol.numberOrNull("24.6 C")!!, 0.001f)
        assertEquals(-3.5f, SmartHiveProtocol.numberOrNull("-3.5")!!, 0.001f)
        assertNull(SmartHiveProtocol.numberOrNull("ERR"))
    }

    @Test
    fun `parses the seven field auto program`() {
        val program = SmartHiveProtocol.parseProgram("1,2,35.00,36.50,0.50,60.00,5.00")!!
        assertEquals(1, program.active)
        assertEquals(2, program.id)
        assertEquals(35f, program.temperatureTarget, 0.001f)
        assertEquals(36.5f, program.fanOnTemperature, 0.001f)
        assertEquals(0.5f, program.fanTemperatureHysteresis, 0.001f)
        assertEquals(60f, program.fanOnHumidity, 0.001f)
        assertEquals(5f, program.fanHumidityHysteresis, 0.001f)
    }

    @Test
    fun `rejects programs with too few fields`() {
        assertNull(SmartHiveProtocol.parseProgram("1,2,35.00"))
        assertNull(SmartHiveProtocol.parseProgram(""))
    }

    @Test
    fun `round trips a program through the wire format`() {
        val text = "0,3,34.50,35.00,0.50,58.00,4.00"
        assertEquals(text, SmartHiveProtocol.formatProgram(SmartHiveProtocol.parseProgram(text)!!))
    }

    @Test
    fun `characteristic uuids are unique`() {
        assertEquals(HiveField.entries.size, HiveField.byUuid.size)
        assertTrue(HiveField.polled.contains(HiveField.TempInside))
    }
}

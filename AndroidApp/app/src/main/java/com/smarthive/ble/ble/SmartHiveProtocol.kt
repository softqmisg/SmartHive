package com.smarthive.ble.ble

import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.UUID

private fun uuid(value: String): UUID = UUID.fromString(value)

/**
 * Characteristics exposed by the SmartHive board. Every value is exchanged as UTF-8 text, so the
 * app reads/writes strings and only parses numbers for the values it charts or edits.
 */
enum class HiveField(
    val uuid: UUID,
    val label: String,
    val unit: String = "",
    val writable: Boolean = false,
    val group: Group = Group.Status,
) {
    TempInside(uuid("735d97aa-84e4-49b8-9c1c-8f9c47f0045f"), "Temperature inside", "°C", group = Group.Live),
    HumInside(uuid("c8abadb3-d645-4b16-a851-d113857bce4e"), "Humidity inside", "%", group = Group.Live),
    TempOutside(uuid("9a170546-0e58-433d-8057-8478b95c0a18"), "Temperature outside", "°C", group = Group.Live),
    HumOutside(uuid("fdda7dfd-79fa-41b6-80d8-ef01f6007360"), "Humidity outside", "%", group = Group.Live),
    HeaterCurrent(uuid("94981336-ac09-48c7-8a62-86ba780bd734"), "Heater current", "A", group = Group.Live),
    HeaterAvgCurrent(uuid("fbf5e2d6-9d02-42c2-a33c-0b5bd6c22727"), "Heater average current", "A", group = Group.Live),

    HiveState(uuid("c89076e1-7f68-4f6d-8a55-c4956c8da8e1"), "Hive state", writable = true, group = Group.Control),
    HeaterState(uuid("7b91fb80-a20d-4a97-b2ad-48728db985f8"), "Heater state", writable = true, group = Group.Control),
    FanState(uuid("03a51ab4-0b51-4db3-8783-f7e350c847a1"), "Fan state", writable = true, group = Group.Control),
    AutoProgram(uuid("fdcbcf24-2aff-4ab4-a6bb-a656ce0dce0a"), "Auto program (setpoints)", writable = true, group = Group.Control),
    SterilProgram(uuid("04231659-8ce4-4a5e-a246-af8fb502c4b8"), "Sterilisation program", group = Group.Hidden),
    PidCoeff(uuid("e887f503-36fd-463d-b4d6-77f024094347"), "PID coefficients", writable = true, group = Group.Control),
    PidState(uuid("a1a7eb92-db3a-4714-82fb-7f953ced4264"), "PID state", group = Group.Control),
    LoadDefault(uuid("ed930b8d-6932-4e0a-962d-bbc2eba56d2c"), "Load defaults", writable = true, group = Group.Control),

    DeviceName(uuid("dc4a6c5f-8d4a-4e4a-9dbe-2f7aad811f80"), "Device name", writable = true, group = Group.Settings),
    Position(uuid("bcd72712-7705-49c5-b2a8-bbc0eb72a988"), "Position", writable = true, group = Group.Settings),
    Time(uuid("fb75341e-8ab1-4355-9b69-39471f6be25d"), "Time", writable = true, group = Group.Settings),
    Date(uuid("ae928490-34aa-4e9c-be83-0ca678f5eda2"), "Date", writable = true, group = Group.Settings),
    BlePassword(uuid("f1120eac-310b-4ffe-93c3-1d8aba7c58f5"), "BLE password", group = Group.Hidden),
    SunTime(uuid("ededa9b6-a8f3-4f1b-8d20-bc3903499a91"), "Sun time", group = Group.Settings),

    SerialNumber(uuid("734447d3-2cc6-4c0d-8eb6-ad942a931d01"), "Serial number", group = Group.Info),
    SoftwareVersion(uuid("a628a3d6-51b4-44c2-b2d1-37c9b8be6972"), "Software version", group = Group.Info),
    HardwareVersion(uuid("00ce255b-d0ca-42ec-8ff2-7df81cfa3f52"), "Hardware version", group = Group.Info),
    SensorInside(uuid("4dff8580-be85-45b8-bb8c-8d0477a4d3be"), "Sensor inside", group = Group.Diagnostics),
    SensorOutside(uuid("659cbe7c-0017-4625-987d-4c8c7a44a30a"), "Sensor outside", group = Group.Diagnostics),
    RtcStatus(uuid("edc7e652-5627-42e1-b9aa-68db0ed33568"), "RTC status", group = Group.Diagnostics),
    SdCard(uuid("d5cc63cf-0f29-4fef-bf2c-94334d1a7aa3"), "SD card", group = Group.Diagnostics),
    CheckHeater(uuid("12cf5269-61e8-4201-a165-a5adeae5f83f"), "Heater self-test", group = Group.Diagnostics),
    CheckFan(uuid("f4fb3847-3532-4ad3-a58d-14b991b71fdd"), "Fan self-test", group = Group.Diagnostics),
    CheckSt20Inside(uuid("e02f6d38-6c4a-4389-8c9c-e980d522a491"), "ST20 inside self-test", group = Group.Diagnostics),
    CheckSt20Outside(uuid("85d9ed33-04a8-4b97-ac32-1a046ee871bc"), "ST20 outside self-test", group = Group.Diagnostics),
    ;

    /** [Hidden] fields exist on the board but are deliberately not exposed by the app. */
    enum class Group { Live, Control, Settings, Info, Diagnostics, Status, Hidden }

    companion object {
        val byUuid: Map<UUID, HiveField> = entries.associateBy { it.uuid }

        /** Polled on a timer while connected, for boards that do not notify. */
        val polled: List<HiveField> = entries.filter { it.group == Group.Live } +
            listOf(HeaterState, FanState, HiveState, PidState, AutoProgram)

        val numeric: Set<HiveField> = setOf(
            TempInside, HumInside, TempOutside, HumOutside, HeaterCurrent, HeaterAvgCurrent,
        )
    }
}

object SmartHiveProtocol {

    val CCCD: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

    const val NAME_PREFIX = "SmartHive"

    private val namePattern = Regex("^SmartHive(\\d+)$")

    fun deviceIdOrNull(name: String?): String? =
        name?.trim()?.let { namePattern.find(it)?.groupValues?.get(1) }

    fun decode(value: ByteArray): String = value.toString(Charsets.UTF_8).trim().trim('\u0000')

    fun encode(text: String): ByteArray = text.toByteArray(Charsets.UTF_8)

    /** Extracts the first number from a text value such as `24.6`, `24.6 C` or `T=24.6`. */
    fun numberOrNull(text: String?): Float? {
        if (text.isNullOrBlank()) return null
        return Regex("-?\\d+(\\.\\d+)?").find(text)?.value?.toFloatOrNull()
    }

    /**
     * Auto/sterilisation programs are comma separated:
     * `active,id,temperatureTarget,fanOnTemperature,fanTHysteresis,fanOnHumidity,fanHHysteresis`.
     */
    fun parseProgram(text: String?): HiveProgram? {
        if (text.isNullOrBlank()) return null
        val parts = text.split(',').map { it.trim() }
        if (parts.size < 7) return null
        return HiveProgram(
            active = numberOrNull(parts[0])?.toInt() ?: 0,
            id = numberOrNull(parts[1])?.toInt() ?: 0,
            temperatureTarget = numberOrNull(parts[2]) ?: 0f,
            fanOnTemperature = numberOrNull(parts[3]) ?: 0f,
            fanTemperatureHysteresis = numberOrNull(parts[4]) ?: 0f,
            fanOnHumidity = numberOrNull(parts[5]) ?: 0f,
            fanHumidityHysteresis = numberOrNull(parts[6]) ?: 0f,
        )
    }

    fun formatProgram(program: HiveProgram): String = listOf(
        program.active.toString(),
        program.id.toString(),
        wire(program.temperatureTarget),
        wire(program.fanOnTemperature),
        wire(program.fanTemperatureHysteresis),
        wire(program.fanOnHumidity),
        wire(program.fanHumidityHysteresis),
    ).joinToString(",")

    /** Always dot-decimal, whatever the phone locale is. */
    private fun wire(value: Float) = String.format(Locale.US, "%.2f", value)

    /** Splits a comma separated value into [count] trimmed parts, padding with empty strings. */
    fun parts(text: String?, count: Int): List<String> {
        val parts = text.orEmpty().split(',').map { it.trim() }
        return List(count) { parts.getOrElse(it) { "" } }
    }

    private val timePattern = Regex("^\\d{1,2}:\\d{1,2}:\\d{1,2}$")
    private val datePattern = Regex("^\\d{4}-\\d{1,2}-\\d{1,2}$")

    fun isTime(text: String) = timePattern.matches(text.trim())

    fun isDate(text: String) = datePattern.matches(text.trim())

    fun isHiveName(text: String) = deviceIdOrNull(text) != null

    fun now(): String = SimpleDateFormat("HH:mm:ss", Locale.US).format(Date())

    fun today(): String = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Date())
}

data class HiveProgram(
    val active: Int,
    val id: Int,
    val temperatureTarget: Float,
    val fanOnTemperature: Float,
    val fanTemperatureHysteresis: Float,
    val fanOnHumidity: Float,
    val fanHumidityHysteresis: Float,
) {
    companion object {
        val Default = HiveProgram(
            active = 1,
            id = 0,
            temperatureTarget = 35f,
            fanOnTemperature = 36f,
            fanTemperatureHysteresis = 0.5f,
            fanOnHumidity = 60f,
            fanHumidityHysteresis = 5f,
        )
    }
}

package com.smarthive.ble.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilterChip
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.smarthive.ble.MainViewModel
import com.smarthive.ble.ble.ConnectionState
import com.smarthive.ble.ble.HiveField
import com.smarthive.ble.ble.HiveProgram
import com.smarthive.ble.ble.SmartHiveProtocol

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DeviceScreen(viewModel: MainViewModel, onBack: () -> Unit) {
    val device by viewModel.selectedDevice.collectAsStateWithLifecycle()
    val state by viewModel.connectionState.collectAsStateWithLifecycle()
    val values by viewModel.values.collectAsStateWithLifecycle()
    val history by viewModel.history.collectAsStateWithLifecycle()
    val autoProgram by viewModel.autoProgram.collectAsStateWithLifecycle()

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(device?.name ?: "Device") },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(Icons.AutoMirrored.Filled.ArrowBack, contentDescription = "Back")
                    }
                },
                actions = {
                    IconButton(onClick = viewModel::refresh) {
                        Icon(Icons.Default.Refresh, contentDescription = "Read values")
                    }
                },
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(16.dp)
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(16.dp),
        ) {
            Text(
                when (state) {
                    ConnectionState.Connecting -> "Connecting…"
                    ConnectionState.Connected -> "Connected · ${values.size} characteristics"
                    ConnectionState.Failed -> "Connection failed"
                    ConnectionState.Disconnected -> "Disconnected"
                },
                style = MaterialTheme.typography.labelLarge,
            )

            Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                ReadingCard("Inside temp", values[HiveField.TempInside], "°C", Modifier.weight(1f))
                ReadingCard("Inside humidity", values[HiveField.HumInside], "%", Modifier.weight(1f))
            }
            Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                ReadingCard("Outside temp", values[HiveField.TempOutside], "°C", Modifier.weight(1f))
                ReadingCard("Outside humidity", values[HiveField.HumOutside], "%", Modifier.weight(1f))
            }

            Card {
                Column(Modifier.padding(16.dp)) {
                    Text("Inside temperature vs time", style = MaterialTheme.typography.titleMedium)
                    Spacer(Modifier.height(8.dp))
                    TemperatureChart(
                        samples = history,
                        targetTemperature = autoProgram?.temperatureTarget,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(220.dp),
                    )
                }
            }

            ProgramCard(program = autoProgram, onApply = viewModel::writeAutoProgram)

            ControlCard(values = values, onWrite = viewModel::write)

            InfoCard(
                title = "Board",
                fields = HiveField.entries.filter { it.group == HiveField.Group.Info },
                values = values,
            )
            InfoCard(
                title = "Diagnostics",
                fields = HiveField.entries.filter {
                    it.group == HiveField.Group.Diagnostics || it.group == HiveField.Group.Settings
                },
                values = values,
            )
        }
    }
}

@Composable
private fun ReadingCard(
    label: String,
    rawValue: String?,
    unit: String,
    modifier: Modifier = Modifier,
) {
    val number = SmartHiveProtocol.numberOrNull(rawValue)
    Card(modifier) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(label, style = MaterialTheme.typography.labelMedium)
            Text(
                number?.let { "%.1f%s".format(it, unit) } ?: rawValue ?: "—",
                style = MaterialTheme.typography.headlineSmall,
            )
        }
    }
}

/**
 * Editor for the auto program, whose wire format is
 * `active,id,temperatureTarget,fanOnTemperature,fanTHysteresis,fanOnHumidity,fanHHysteresis`.
 * Fields not touched here are sent back unchanged.
 */
@Composable
private fun ProgramCard(program: HiveProgram?, onApply: (HiveProgram) -> Unit) {
    val board = program ?: HiveProgram.Default
    var draft by remember(program == null, program?.id) { mutableStateOf(board) }

    Card {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp),
        ) {
            Text("Auto program", style = MaterialTheme.typography.titleMedium)
            Text(
                if (program == null) "not read from the board yet"
                else "board: target %.1f°C · fan on %.1f°C / %.1f%% · program #%d · %s".format(
                    program.temperatureTarget,
                    program.fanOnTemperature,
                    program.fanOnHumidity,
                    program.id,
                    if (program.active != 0) "active" else "inactive",
                ),
                style = MaterialTheme.typography.labelSmall,
            )

            ProgramSlider(
                label = "Target temperature",
                unit = "°C",
                value = draft.temperatureTarget,
                range = 0f..60f,
                onValueChange = { draft = draft.copy(temperatureTarget = it) },
            )
            ProgramSlider(
                label = "Fan on humidity",
                unit = "%",
                value = draft.fanOnHumidity,
                range = 0f..100f,
                onValueChange = { draft = draft.copy(fanOnHumidity = it) },
            )
            ProgramSlider(
                label = "Fan on temperature",
                unit = "°C",
                value = draft.fanOnTemperature,
                range = 0f..60f,
                onValueChange = { draft = draft.copy(fanOnTemperature = it) },
            )
            ProgramSlider(
                label = "Temperature hysteresis",
                unit = "°C",
                value = draft.fanTemperatureHysteresis,
                range = 0f..10f,
                onValueChange = { draft = draft.copy(fanTemperatureHysteresis = it) },
            )
            ProgramSlider(
                label = "Humidity hysteresis",
                unit = "%",
                value = draft.fanHumidityHysteresis,
                range = 0f..20f,
                onValueChange = { draft = draft.copy(fanHumidityHysteresis = it) },
            )

            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp),
            ) {
                Text("Program active", modifier = Modifier.weight(1f))
                Switch(
                    checked = draft.active != 0,
                    onCheckedChange = { draft = draft.copy(active = if (it) 1 else 0) },
                )
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End,
            ) {
                Button(onClick = { onApply(draft) }) { Text("Send to board") }
            }
        }
    }
}

@Composable
private fun ProgramSlider(
    label: String,
    unit: String,
    value: Float,
    range: ClosedFloatingPointRange<Float>,
    onValueChange: (Float) -> Unit,
) {
    Text("$label: %.1f%s".format(value, unit), style = MaterialTheme.typography.bodyMedium)
    Slider(value = value, onValueChange = onValueChange, valueRange = range)
}

@Composable
private fun ControlCard(
    values: Map<HiveField, String>,
    onWrite: (HiveField, String) -> Unit,
) {
    val quickFields = setOf(
        HiveField.AutoProgram,
        HiveField.HiveState,
        HiveField.HeaterState,
        HiveField.FanState,
        HiveField.LoadDefault,
    )
    val writableFields = HiveField.entries.filter { it.writable && it !in quickFields }

    Card {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Text("Controls", style = MaterialTheme.typography.titleMedium)

            ChoiceRow(
                label = "Hive state",
                options = listOf("AUTO", "MANUAL", "STERIL"),
                current = values[HiveField.HiveState],
                onSelect = { onWrite(HiveField.HiveState, it) },
            )
            ChoiceRow(
                label = "Heater",
                options = listOf("ON", "OFF"),
                current = values[HiveField.HeaterState],
                onSelect = { onWrite(HiveField.HeaterState, it) },
            )
            ChoiceRow(
                label = "Fan",
                options = listOf("ON", "OFF"),
                current = values[HiveField.FanState],
                onSelect = { onWrite(HiveField.FanState, it) },
            )

            Text(
                "Values below are plain text, exactly as the board expects them.",
                style = MaterialTheme.typography.labelSmall,
            )
            writableFields.forEach { field ->
                WritableField(field = field, current = values[field], onWrite = onWrite)
            }
            OutlinedButton(onClick = { onWrite(HiveField.LoadDefault, "1") }) {
                Text("Load board defaults")
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun ChoiceRow(
    label: String,
    options: List<String>,
    current: String?,
    onSelect: (String) -> Unit,
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Text(label, style = MaterialTheme.typography.bodyMedium, modifier = Modifier.weight(1f))
        options.forEach { option ->
            val selected = current?.trim()?.equals(option, ignoreCase = true) == true
            FilterChip(
                selected = selected,
                onClick = { onSelect(option) },
                label = { Text(option) },
            )
        }
    }
}

@Composable
private fun WritableField(
    field: HiveField,
    current: String?,
    onWrite: (HiveField, String) -> Unit,
) {
    var draft by remember(current) { mutableStateOf(current.orEmpty()) }
    Row(
        modifier = Modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        OutlinedTextField(
            value = draft,
            onValueChange = { draft = it },
            label = { Text(field.label) },
            singleLine = true,
            modifier = Modifier.weight(1f),
        )
        OutlinedButton(onClick = { onWrite(field, draft) }) { Text("Set") }
    }
}

@Composable
private fun InfoCard(
    title: String,
    fields: List<HiveField>,
    values: Map<HiveField, String>,
) {
    val present = fields.filter { values.containsKey(it) }
    if (present.isEmpty()) return
    Card {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(6.dp),
        ) {
            Text(title, style = MaterialTheme.typography.titleMedium)
            present.forEach { field ->
                Row(Modifier.fillMaxWidth()) {
                    Text(
                        field.label,
                        style = MaterialTheme.typography.bodySmall,
                        modifier = Modifier.weight(1f),
                    )
                    Text(values[field].orEmpty(), style = MaterialTheme.typography.bodySmall)
                }
            }
        }
    }
}

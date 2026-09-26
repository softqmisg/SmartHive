package com.smarthive.ble.ui

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.unit.dp
import com.smarthive.ble.Sample
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/** Temperature-vs-time line chart with a target line, drawn directly on a Canvas. */
@Composable
fun TemperatureChart(
    samples: List<Sample>,
    targetTemperature: Float?,
    modifier: Modifier = Modifier,
) {
    val lineColor = MaterialTheme.colorScheme.primary
    val targetColor = MaterialTheme.colorScheme.secondary
    val axisColor = MaterialTheme.colorScheme.outline
    val labelColor = MaterialTheme.colorScheme.onSurfaceVariant
    val timeFormat = remember { SimpleDateFormat("HH:mm:ss", Locale.getDefault()) }

    Box(modifier, contentAlignment = Alignment.Center) {
        if (samples.size < 2) {
            Text("Waiting for readings…", style = MaterialTheme.typography.bodySmall)
            return@Box
        }

        Canvas(Modifier.fillMaxSize()) {
            val leftPadding = 44.dp.toPx()
            val bottomPadding = 22.dp.toPx()
            val topPadding = 10.dp.toPx()
            val rightPadding = 8.dp.toPx()
            val plotWidth = size.width - leftPadding - rightPadding
            val plotHeight = size.height - topPadding - bottomPadding

            val temperatures = samples.map { it.temperature }
            var minValue = minOf(temperatures.min(), targetTemperature ?: temperatures.min())
            var maxValue = maxOf(temperatures.max(), targetTemperature ?: temperatures.max())
            if (maxValue - minValue < 1f) {
                val mid = (maxValue + minValue) / 2f
                minValue = mid - 0.5f
                maxValue = mid + 0.5f
            }
            val padding = (maxValue - minValue) * 0.1f
            minValue -= padding
            maxValue += padding

            val firstTime = samples.first().timestampMs
            val span = (samples.last().timestampMs - firstTime).coerceAtLeast(1L)

            fun x(sample: Sample) =
                leftPadding + plotWidth * (sample.timestampMs - firstTime) / span.toFloat()

            fun y(value: Float) =
                topPadding + plotHeight * (1f - (value - minValue) / (maxValue - minValue))

            drawLine(
                color = axisColor,
                start = Offset(leftPadding, topPadding),
                end = Offset(leftPadding, topPadding + plotHeight),
                strokeWidth = 2f,
            )
            drawLine(
                color = axisColor,
                start = Offset(leftPadding, topPadding + plotHeight),
                end = Offset(leftPadding + plotWidth, topPadding + plotHeight),
                strokeWidth = 2f,
            )

            val textPaint = android.graphics.Paint().apply {
                color = labelColor.toArgb()
                textSize = 26f
                isAntiAlias = true
            }
            listOf(maxValue, (maxValue + minValue) / 2f, minValue).forEach { value ->
                val yPos = y(value)
                drawLine(
                    color = axisColor.copy(alpha = 0.25f),
                    start = Offset(leftPadding, yPos),
                    end = Offset(leftPadding + plotWidth, yPos),
                    strokeWidth = 1f,
                )
                drawContext.canvas.nativeCanvas.drawText(
                    "%.1f".format(value),
                    4f,
                    yPos + 8f,
                    textPaint,
                )
            }

            val nativeCanvas = drawContext.canvas.nativeCanvas
            nativeCanvas.drawText(
                timeFormat.format(Date(samples.first().timestampMs)),
                leftPadding,
                size.height - 4f,
                textPaint,
            )
            nativeCanvas.drawText(
                timeFormat.format(Date(samples.last().timestampMs)),
                size.width - 130f,
                size.height - 4f,
                textPaint,
            )

            targetTemperature?.let { target ->
                val yPos = y(target)
                drawLine(
                    color = targetColor,
                    start = Offset(leftPadding, yPos),
                    end = Offset(leftPadding + plotWidth, yPos),
                    strokeWidth = 3f,
                    pathEffect = PathEffect.dashPathEffect(floatArrayOf(14f, 10f)),
                )
            }

            val path = Path()
            samples.forEachIndexed { index, sample ->
                val point = Offset(x(sample), y(sample.temperature))
                if (index == 0) path.moveTo(point.x, point.y) else path.lineTo(point.x, point.y)
            }
            drawPath(path, color = lineColor, style = Stroke(width = 4f))

            val last = samples.last()
            drawCircle(
                color = lineColor,
                radius = 7f,
                center = Offset(x(last), y(last.temperature)),
            )
        }
    }
}

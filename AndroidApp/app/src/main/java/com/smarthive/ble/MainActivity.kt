package com.smarthive.ble

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.graphics.Color
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.smarthive.ble.ui.DeviceScreen
import com.smarthive.ble.ui.LoginScreen
import com.smarthive.ble.ui.ScanScreen

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent { SmartHiveApp() }
    }
}

private val HoneyLight = lightColorScheme(
    primary = Color(0xFFB8860B),
    secondary = Color(0xFF3F6C51),
)

private val HoneyDark = darkColorScheme(
    primary = Color(0xFFE0B341),
    secondary = Color(0xFF7FC39B),
)

@Composable
fun SmartHiveApp(viewModel: MainViewModel = viewModel()) {
    MaterialTheme(colorScheme = if (isSystemInDarkTheme()) HoneyDark else HoneyLight) {
        Surface {
            val navController = rememberNavController()
            val user by viewModel.loggedInUser.collectAsStateWithLifecycle()

            NavHost(navController = navController, startDestination = "login") {
                composable("login") {
                    LoginScreen(viewModel) { navController.navigate("scan") }
                }
                composable("scan") {
                    ScanScreen(
                        viewModel = viewModel,
                        onDeviceSelected = { navController.navigate("device") },
                        onLogout = {
                            viewModel.logout()
                            navController.navigate("login") { popUpTo("login") { inclusive = true } }
                        },
                    )
                }
                composable("device") {
                    DeviceScreen(viewModel) {
                        viewModel.disconnect()
                        navController.popBackStack()
                    }
                }
            }

            if (user == null && navController.currentDestination?.route == "device") {
                navController.navigate("login") { popUpTo("login") { inclusive = true } }
            }
        }
    }
}

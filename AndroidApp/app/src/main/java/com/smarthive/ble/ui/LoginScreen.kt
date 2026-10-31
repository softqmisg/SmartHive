package com.smarthive.ble.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.smarthive.ble.MainViewModel

@Composable
fun LoginScreen(viewModel: MainViewModel, onLoggedIn: () -> Unit) {
    var username by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }
    var registering by remember { mutableStateOf(false) }
    val error by viewModel.error.collectAsStateWithLifecycle()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Text("SmartHive", style = MaterialTheme.typography.displaySmall)
        Text(
            if (registering) "Create an account" else "Sign in to manage your hive",
            style = MaterialTheme.typography.bodyMedium,
            textAlign = TextAlign.Center,
        )
        Spacer(Modifier.height(32.dp))

        OutlinedTextField(
            value = username,
            onValueChange = {
                username = it
                viewModel.clearError()
            },
            label = { Text("Username") },
            singleLine = true,
            modifier = Modifier.fillMaxWidth(),
            keyboardOptions = KeyboardOptions(imeAction = ImeAction.Next),
        )
        Spacer(Modifier.height(12.dp))
        OutlinedTextField(
            value = password,
            onValueChange = {
                password = it
                viewModel.clearError()
            },
            label = { Text("Password") },
            singleLine = true,
            visualTransformation = PasswordVisualTransformation(),
            modifier = Modifier.fillMaxWidth(),
            keyboardOptions = KeyboardOptions(imeAction = ImeAction.Done),
        )

        if (error != null) {
            Spacer(Modifier.height(8.dp))
            Text(
                error.orEmpty(),
                color = MaterialTheme.colorScheme.error,
                style = MaterialTheme.typography.bodySmall,
            )
        }

        Spacer(Modifier.height(20.dp))
        Button(
            onClick = {
                val ok = if (registering) {
                    viewModel.register(username, password) && viewModel.login(username, password)
                } else {
                    viewModel.login(username, password)
                }
                if (ok) onLoggedIn()
            },
            enabled = username.isNotBlank() && password.isNotBlank(),
            modifier = Modifier.fillMaxWidth(),
        ) {
            Text(if (registering) "Create account" else "Log in")
        }

        TextButton(onClick = {
            registering = !registering
            viewModel.clearError()
        }) {
            Text(if (registering) "I already have an account" else "Create an account")
        }

        Spacer(Modifier.height(24.dp))
        Text(
            "Demo credentials: ${viewModel.defaultCredentialsHint}",
            style = MaterialTheme.typography.labelSmall,
        )
    }
}

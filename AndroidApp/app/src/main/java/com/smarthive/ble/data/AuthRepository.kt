package com.smarthive.ble.data

import android.content.Context
import java.security.MessageDigest

/**
 * Offline credential store. The first account is seeded so the app is usable out of the box;
 * swap this class for an API client when the backend exists.
 */
class AuthRepository(context: Context) {

    private val prefs = context.getSharedPreferences("smarthive-auth", Context.MODE_PRIVATE)

    init {
        if (!prefs.contains(key(DEFAULT_USER))) {
            prefs.edit().putString(key(DEFAULT_USER), hash(DEFAULT_PASSWORD)).apply()
        }
    }

    fun login(username: String, password: String): Boolean {
        val stored = prefs.getString(key(username.trim().lowercase()), null) ?: return false
        return stored == hash(password)
    }

    fun register(username: String, password: String): Boolean {
        val normalized = username.trim().lowercase()
        if (normalized.isEmpty() || password.length < 4) return false
        if (prefs.contains(key(normalized))) return false
        prefs.edit().putString(key(normalized), hash(password)).apply()
        return true
    }

    private fun key(username: String) = "user:${username.trim().lowercase()}"

    private fun hash(password: String): String =
        MessageDigest.getInstance("SHA-256")
            .digest(password.toByteArray())
            .joinToString("") { "%02x".format(it) }

    companion object {
        const val DEFAULT_USER = "admin"
        const val DEFAULT_PASSWORD = "smarthive"
    }
}

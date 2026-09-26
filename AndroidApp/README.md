# SmartHive Android app

Android client for the SmartHive board's BLE GATT interface
(`SmatHiveClient/Firmware/.../src/UserBLE.h`).

## Features

- Login screen (local accounts, default `admin` / `smarthive`).
- Scan that only lists advertisers named `SmartHiveXXXX` (exactly four id characters).
- Connect and read every known characteristic as UTF-8 text: inside/outside temperature and
  humidity, heater current, states, versions, diagnostics.
- Auto program editor over `HIVE_AUTOPROG_UUID`
  (`active,id,TemperatureTarget,fanOnTemperature,fanTHysteresis,fanOnHumidity,fanHHysteresis`);
  fields that are not edited are written back unchanged.
- Hive state (`AUTO`/`MANUAL`/`STERIL`), heater and fan `ON`/`OFF` controls, plus a text editor
  for the remaining writable characteristics (position, time, date, PID, sterilisation program…).
- Inside temperature vs time chart with the target temperature drawn as a reference line.
- A "simulated device" entry in the scan list for trying the UI without hardware.

## Build

```bash
echo "sdk.dir=/path/to/android-sdk" > local.properties
./gradlew :app:assembleDebug :app:testDebugUnitTest
```

The APK lands in `app/build/outputs/apk/debug/app-debug.apk`.

## Notes on the firmware protocol

- The read/write characteristics have no notify property, so the app polls them every 3 s; the
  read-only characteristics are notified by the board and are also read once on connect.
- The board exposes a `0x2903` (user description) descriptor but no `0x2902` CCCD, so the app
  only registers for notifications locally and does not treat a missing CCCD as an error.
- Pairing/encryption is compiled out in the firmware (`ENABLE_PAIR_ENCRYPTION 0`), so no PIN is
  required.
- Numbers are always written dot-decimal regardless of the phone locale.

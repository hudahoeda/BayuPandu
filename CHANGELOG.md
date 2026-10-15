# Changelog

All notable changes to BayuPandu are recorded here. Versions follow
[Semantic Versioning](https://semver.org/); the firmware version lives in
`firmware/src/version.h`, and each release is tagged `v<version>`.

## [0.1.0] - 2026-10-15

First tagged version. It runs in the desktop simulator; it has not flown, and
the sensor drivers are still stubs, so it cannot yet be used in the air.

### Added
- Road-sign flight UI on the 240x320 TFT: Flight, Thermal, Navigation, Status
  and Settings pages, a yellow warning footer and a full error sign.
- English and Bahasa Indonesia labels, switchable in Settings.
- Flight statistics: 20 s average climb, glide ratio, airborne time, distance
  and bearing to takeoff, thermal gain and time.
- Two-state Kalman filter for barometric altitude and climb rate, with climb
  tone and sink alarm.
- Desktop simulator (`firmware/sim/`) replaying IGC files, with headless
  screenshots and forced warning states (no GPS, data lost, low battery).
- Host unit tests (`pio test -e native`).
- Firmware version shown on the Settings page.

### Known gaps
- Barometer, GPS, IMU, storage and power drivers are stubs.
- IGC flight logging is not written yet.
- The pin map in `config.h` does not suit the ESP32-C3 build target.

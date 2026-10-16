# Changelog

All notable changes to BayuPandu are recorded here. Versions follow
[Semantic Versioning](https://semver.org/); the firmware version lives in
`firmware/src/version.h`, and each release is tagged `v<version>`.

## [Unreleased]

### Added
- BayuPandu mark: a paraglider wing carried on wind lines, drawn as vector on the
  device and generated as SVG/PNG in `assets/brand/` (lockups, one-colour
  versions, favicon, social preview). The README header uses it.
- Boot sign: wind meets a ridge and lifts the wing while the self-test shows
  barometer, GPS, card and battery; a failed check is a yellow line. Any button
  skips it.
- Startup chime timed to the boot sign: a rising wind run, a vario climb
  double-beep a step higher for each passed check, a low sink tone for a failed
  one, then a rising (or, after a failure, falling) "ready". Written as MIDI
  notes; `assets/sound/` holds it as `.mid` and `.wav`.
- Buzzer driver (`AudioImpl`) plays tones through the Arduino core's `tone()`.
- Pinout for the ESP32-S3 in `documentation/pinout.md`, with an SD card chip
  select. `config.h` refuses to build if a pin is a strap, flash, PSRAM, USB or
  console pin, or if two functions share one.
- Simulator: sound through SDL (`--mute` to silence), `--export-chime DIR`,
  `--boot-frames DIR` and `--gps-silent`; `--shots` also saves the boot sign.
- Simulator `--trace FILE`: the replay's climb from the log next to the vario
  filter's, as 10 Hz CSV, for checking and tuning the filter.

- IGC flight log on the microSD card, with no buttons to press: it starts at
  takeoff with the minute before it included, writes a record a second, and
  closes a minute after landing. Pilot and glider for the header come from
  `pilot.txt` on the card. The Status page shows READY, REC with the time
  recording, NO CARD or WRITE FAILED. See `documentation/flight-log.md`.
- SD card driver for the device (`SdStorage`, sharing the display's SPI bus);
  each log gets the device's own 3-character id from its chip.
- Simulator: replays log to `firmware/sim/flights/` (`--card DIR` for another
  folder), and the replay's GPS carries the date from the IGC header.

### Changed
- The firmware targets the ESP32-S3 (`pio run -e esp32s3`, any S3 board)
  instead of the ESP32-C3, whose pins could not fit the parts list. The C3 and
  `picsimlab` envs are gone, and `pio run` builds the S3 by default.
- A replay now goes through the real vario: a replay barometer turns the log
  into pressure with MS5611-like noise and feeds `VariometerService`, so the
  climb, altitude, vario tone and BARO self-test in the simulator are what the
  device would show. Between fixes the altitude follows a smooth curve instead
  of the log's whole-metre steps.
- The vario sounds like an XC vario: in lift, beeps that rise in pitch and
  quicken with the climb (about 2 a second at 0.5 m/s, 8 at 5 m/s); in sink
  past -2.5 m/s, one steady low tone. It used to be a single continuous tone.
- The simulator's buzzer is band-limited (PolyBLEP, 44.1 kHz): the plain square
  wave folded its high harmonics back as a faint off-key undertone under each
  beep. The chime `.wav` files are rendered again with it.
- A replay no longer raises a yellow "SIMULATION ACTIVE" footer; yellow is kept
  for trouble.

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

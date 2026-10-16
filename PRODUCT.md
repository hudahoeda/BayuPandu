# Product

<!-- impeccable:product-schema 1 -->

## Platform

embedded

(Outside the web/ios/android/adaptive set: firmware UI on an ESP32 with LVGL 8.3. Not a web or mobile app.)

## Users

Paraglider pilots flying **cross-country (XC)**, mainly in the Indonesian paragliding community. In the air they read the device in short glances between thermals and while circling, with gloves on and their attention outside. Secondary audiences: DIY electronics hobbyists who build the device themselves, and competition pilots who need IGC logs.

## Product Purpose

BayuPandu is an open-source paragliding flight computer: variometer, altimeter, GPS and IGC flight logger in one self-built unit, costing under Rp 1–2 million in parts bought locally. The display has to give an XC pilot climb, altitude and glide information at a glance, in direct tropical sun, without pulling their eyes off the sky for long. Success means a pilot trusts it as a secondary instrument on a real XC flight.

## Positioning

A fully open, locally buildable instrument: firmware, hardware and case are open source, with parts sourced from Tokopedia/Shopee. It's meant to be repairable and replicable rather than a sealed consumer product.

## Operating Context

- Mounted in a **cockpit / harness pod**, about 50–70 cm from the eyes, glanced down at.
- Bright outdoor light, often direct sun; vibration; gloves.
- Audio vario tones carry most in-thermal feedback; the screen backs them up and adds context (glide, height, direction).
- Controlled with **four physical buttons** (UP, DOWN, ENTER, CANCEL). No touchscreen.
- Development happens in the desktop simulator (`firmware/sim/`), replaying IGC files.

## Capabilities and Constraints

- Display: **240×320 portrait colour TFT (ILI9341)** for now. A sunlight-readable panel (transflective TFT or Garmin-style memory-in-pixel) may come later, so the UI must not depend on backlight brightness or subtle colours.
- LVGL 8.3 on the ESP32-S3 (any board; pins in `documentation/pinout.md`): limited RAM (LVGL heap 64 KB; one full set of pages uses about 48 KB) and bitmap fonts only, generated at fixed sizes by `firmware/tools/gen_fonts.sh`.
- Data available in `FlightData`: baro altitude, vertical speed, GPS position, speed, heading, satellites, HDOP, attitude, battery.
- Undecided: final display panel; which XC navigation features to include (wind, waypoints, distance to takeoff); where the IMU fits in.

## Brand Commitments

- Name: **BayuPandu**.
- UI language: **English and Bahasa Indonesia, switchable**, with English as the default and standard aviation abbreviations.

## Evidence on Hand

- Example flight: `documentation/example/complex_example_lxn.igc` (an LXNAV glider log used for replay).
- No pilot testimonials, flight test results or benchmarks exist yet. Don't invent any.

## Product Principles

1. **Glanceable over complete.** In flight, one look must answer "am I climbing, how high, can I make the glide?" Everything else is secondary.
2. **Readable in the worst light.** Design for direct tropical sun on a cheap panel; if it only reads indoors, it fails.
3. **Never ambiguous about safety.** Low battery, lost GPS and stale data must be impossible to miss, and must never be shown as if they were good data.
4. **Buildable and hackable.** Plain, documented code a hobbyist can change; no cleverness that costs RAM or clarity.

## Accessibility & Inclusion

Bilingual labels (EN/ID). Colour must never be the only signal: lift and sink, and warnings, also differ in shape or position, for colour-blind pilots and for monochrome panels.

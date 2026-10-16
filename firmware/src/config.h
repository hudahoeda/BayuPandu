#pragma once

#include <Arduino.h>

// PINS: ESP32-S3, any module or dev board (documentation/pinout.md).
// Every pin below is free on all S3 boards. Never used: 0, 3, 45, 46 (boot
// straps), 19/20 (USB), 26-32 (SPI flash), 33-37 (octal PSRAM on N8R8/N16R8
// modules), 38 and 48 (on-board RGB LED on DevKitC-1), 43/44 (UART0 console).

// BUTTONS (to GND, internal pull-up)
#define PIN_UP 4
#define PIN_DOWN 5
#define PIN_ENTER 6
#define PIN_CANCEL 7

// I2C: MS5611 barometer, IMU
#define PIN_SDA 8
#define PIN_SCL 9

// GPS on UART1 (ESP TX -> GPS RX, GPS TX -> ESP RX)
#define PIN_GPS_TX 17
#define PIN_GPS_RX 18

// BUZZER: passive piezo, driven by LEDC
#define PIN_BUZZER 2

// LED: external status LED (the DevKitC-1's own LED is addressable RGB)
#define PIN_LED_BUILTIN 16

// DISPLAY: ILI9341 on the FSPI default pins; the SD card shares the bus
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   14
#define TFT_RST  21
#define TFT_BL   -1 // No back light control

// SD CARD (SPI, shared with the display)
#define PIN_SD_CS 15

// --- Other configurations ---
#define SERIAL_BAUD_RATE 115200
#define GPS_BAUD_RATE 9600

#define TIME_ZONE_OFFSET_SECONDS 0

#define FLIGHT_START_VARIO_THRESHOLD 0.5
#define FLIGHT_STOP_VARIO_THRESHOLD 0.2
#define FLIGHT_STOP_TIMEOUT_SECONDS 30

#define ALTITUDE_HISTORY_SIZE 10
#define VARIO_SENSITIVITY 0.5

#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.0
// Battery through a 2:1 divider into ADC1 (ADC2 is unusable while WiFi runs)
#define BATTERY_PIN 1

#if defined(CONFIG_IDF_TARGET_ESP32S3)
namespace PinCheck {
constexpr bool reserved(int pin) {
    return pin == 0 || pin == 3 || pin == 45 || pin == 46 || pin == 19 || pin == 20 ||
           (pin >= 26 && pin <= 38) || pin == 43 || pin == 44 || pin == 48;
}
constexpr int kPins[] = {PIN_UP, PIN_DOWN, PIN_ENTER, PIN_CANCEL, PIN_SDA, PIN_SCL,
                         PIN_GPS_TX, PIN_GPS_RX, PIN_BUZZER, PIN_LED_BUILTIN, TFT_MISO,
                         TFT_MOSI, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST, PIN_SD_CS, BATTERY_PIN};
constexpr int kCount = sizeof(kPins) / sizeof(kPins[0]);
// Recursive, because the ESP32 toolchain builds this as C++11.
constexpr bool allFree(int i = 0) {
    return i == kCount || (!reserved(kPins[i]) && allFree(i + 1));
}
constexpr bool unique(int i, int j) {
    return j == kCount || (kPins[i] != kPins[j] && unique(i, j + 1));
}
constexpr bool allDistinct(int i = 0) {
    return i == kCount || (unique(i, i + 1) && allDistinct(i + 1));
}
static_assert(allFree(), "a pin in config.h is a strap, flash, PSRAM, USB or console pin on the ESP32-S3");
static_assert(allDistinct(), "two functions in config.h share a GPIO");
static_assert(BATTERY_PIN >= 1 && BATTERY_PIN <= 10, "BATTERY_PIN must be an ADC1 pin (GPIO 1-10)");
}  // namespace PinCheck
#endif
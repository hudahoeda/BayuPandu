#pragma once

#include <Arduino.h>

// PINS
#define PIN_UP 3
#define PIN_DOWN 1
#define PIN_ENTER 0
#define PIN_CANCEL 10

// I2C
#define PIN_SDA 18
#define PIN_SCL 19

// GPS
#define PIN_GPS_TX 12
#define PIN_GPS_RX 13

// BUZZER
#define PIN_BUZZER 16

// LED
#define PIN_LED_BUILTIN 21

// DISPLAY (TFT_eSPI)
#define TFT_MISO 14
#define TFT_MOSI 7
#define TFT_SCLK 6
#define TFT_CS   5
#define TFT_DC   4
#define TFT_RST  15
#define TFT_BL   -1 // No back light control

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
#define BATTERY_PIN 1
# Pinout: ESP32-S3

BayuPandu runs on an **ESP32-S3**. The pin map in `firmware/src/config.h` uses
only GPIOs that are free on every S3 module, so any S3 dev board works: the
Espressif DevKitC-1 (N8, N8R8, N16R8), the common clones, or a bare
WROOM-1 module. Build with `pio run -e esp32s3`.

| Function | GPIO | Connects to | Notes |
| :-- | --: | :-- | :-- |
| Button UP | 4 | button to GND | internal pull-up |
| Button DOWN | 5 | button to GND | internal pull-up |
| Button ENTER | 6 | button to GND | internal pull-up |
| Button CANCEL | 7 | button to GND | internal pull-up |
| I2C SDA | 8 | MS5611 (GY-63) SDA, IMU SDA | |
| I2C SCL | 9 | MS5611 (GY-63) SCL, IMU SCL | |
| SPI CS (display) | 10 | ILI9341 CS | FSPI default pins |
| SPI MOSI | 11 | ILI9341 MOSI/SDI, SD DI | shared bus |
| SPI SCLK | 12 | ILI9341 SCK, SD SCK | shared bus |
| SPI MISO | 13 | ILI9341 MISO/SDO, SD DO | shared bus |
| Display DC | 14 | ILI9341 D/C | |
| SD card CS | 15 | microSD module CS | |
| Status LED | 16 | LED + resistor to GND | the DevKitC-1's own LED is addressable RGB |
| GPS TX (ESP out) | 17 | GPS RX | UART1 |
| GPS RX (ESP in) | 18 | GPS TX | UART1 |
| Display RST | 21 | ILI9341 RST | |
| Buzzer | 2 | passive piezo, other leg to GND | LEDC tone |
| Battery sense | 1 | middle of a 2:1 divider from the cell | ADC1; ADC2 can't be read while WiFi runs |

The display backlight (ILI9341 LED pin) goes straight to 3V3 for now.

## Pins to leave alone

| GPIO | Why |
| :-- | :-- |
| 0, 3, 45, 46 | boot strapping pins |
| 19, 20 | native USB (used for flashing and `Serial`) |
| 26–32 | SPI flash |
| 33–37 | octal PSRAM on N8R8 / N16R8 modules |
| 38, 48 | on-board RGB LED on DevKitC-1 boards |
| 43, 44 | UART0 console / USB-UART bridge |

`config.h` checks this at compile time: a pin from this list, or two functions
on one pin, stops the build with a `static_assert`.

## Serial

`Serial` goes to the chip's own USB port (`ARDUINO_USB_CDC_ON_BOOT=1`). On a
DevKitC-1 that is the port marked **USB**, not **UART**.

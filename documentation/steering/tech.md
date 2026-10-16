# BayuPandu Technical Stack

## Build System & Development Environment

- **Platform**: PlatformIO with Visual Studio Code
- **Framework**: Arduino framework for ESP32
- **Language**: C++ with Arduino libraries
- **Version Control**: Git with GitHub hosting
- **Testing**: PlatformIO unit testing framework

## Hardware Platform

- **Microcontroller**: ESP32-S3, any board (dual-core 240MHz, FPU, native USB, Wi-Fi/BLE); pins in `documentation/pinout.md`
- **Development Board**: 30-pin ESP32 development board
- **Programming**: USB serial via Arduino IDE/PlatformIO

## Core Components & Libraries

### Sensors
- **Barometric Pressure**: MS5611 (primary) or BME280 (alternative)
- **GPS**: U-blox NEO-6M with external antenna
- **IMU**: MPU-9250 (accelerometer, gyroscope, magnetometer)

### Display Options
- **Transflective LCD**: 2.4" TFT with SPI interface
- **E-Paper**: 2.9" Waveshare e-ink display with SPI

### Storage & Power
- **Data Logging**: MicroSD card module (SPI)
- **Battery**: 18650 Li-ion with TP4056 charging circuit
- **Audio**: Piezo buzzer for variometer tones

## Software Architecture

### Hardware Abstraction Layer (HAL)
- Generic interfaces for all hardware components (✅ Implemented)
- Separate implementations for real hardware vs. simulation (✅ Mock implementations ready)
- Enables unit testing without physical hardware (✅ ArduinoFake integration complete)

### Modern UI Framework
- **LVGL Integration**: Complete modern GUI framework with responsive widgets
- **Cross-platform Support**: Works on both ESP32 hardware and native testing environment
- **Optimized Configuration**: 16-bit color depth, 48KB memory allocation, essential widgets enabled
- **Manual Tick Management**: Custom timing system for reliable cross-platform operation

### Key Libraries (implemented in platformio.ini)
```ini
lib_deps =
    lvgl/lvgl@^8.4.0                                     # LVGL for modern UI framework (✅ Fully integrated)
    bblanchon/ArduinoJson@^6.21.5                        # JSON parsing for configuration
    adafruit/Adafruit GFX Library@^1.12.1                # Graphics primitives
    adafruit/Adafruit ST7735 and ST7789 Library@^1.11.0  # TFT display drivers
    mikalhart/TinyGPSPlus@^1.1.0                         # GPS data parsing
```

## Common Development Commands

### PlatformIO Commands
```bash
# Build the project
/home/huda/.platformio/penv/bin/pio run

# Build specific environment
/home/huda/.platformio/penv/bin/pio run -e esp32s3     # For ESP32-S3 hardware
/home/huda/.platformio/penv/bin/pio run -e native     # For native testing with ArduinoFake

# Upload to ESP32
/home/huda/.platformio/penv/bin/pio run --target upload

# Monitor serial output
/home/huda/.platformio/penv/bin/pio device monitor

# Run unit tests on the host (no hardware needed)
/home/huda/.platformio/penv/bin/pio test -e native

# Run one suite
/home/huda/.platformio/penv/bin/pio test -e native -f test_vario

# Clean build files
/home/huda/.platformio/penv/bin/pio run --target clean
```

### Project Structure Commands
```bash
# Initialize new PlatformIO project
pio project init --board esp32-s3-devkitc-1

# Add library dependency
pio lib install "library-name"

# Update all libraries
pio lib update
```

## File Format Standards

- **Flight Logging**: IGC format compliance for FAI requirements
- **Configuration**: JSON format for settings storage
- **3D Models**: STL for printing, STEP for source CAD files
- **Schematics**: PDF for documentation, native CAD for source

## Development Workflow

1. **Feature Development**: Create feature branch
2. **HAL Interface**: Define generic hardware interface (✅ Core interfaces implemented)
3. **Simulation**: Implement simulated version for testing (✅ Mock implementations complete)
4. **Unit Tests**: Write tests using simulated components (🚧 ArduinoFake integration ready)
5. **Real Implementation**: Implement actual hardware interface (⏳ Next phase)
6. **Hardware Testing**: Test on physical ESP32 with components (⏳ Hardware-dependent)
7. **Integration**: Merge to main branch after validation

### Current Implementation Status
- ✅ **HAL Layer**: All core interfaces defined (IBarometer, IGPS, IIMU, IDisplay, IAudio, IStorage, IPower)
- ✅ **Service Layer**: Core services implemented (VariometerService, GPSService, IMUService, etc.)
- ✅ **Application Layer**: FlightManager, UserInterface, FlightLogger complete
- ✅ **Mock Testing**: ArduinoFake integration for hardware-independent testing
- ✅ **LVGL Integration**: Modern UI framework fully integrated and operational
- ✅ **LVGL UI Implementation**: Complete redesign of all screens using LVGL widgets
- ✅ **Display Driver**: LVGL display and input drivers implemented
- ✅ **Cross-Platform UI**: Same UI code compiles for ESP32 and native testing
- 🚧 **Data Integration**: FlightManager data binding to LVGL UI in progress
- ⏳ **Hardware Implementations**: Physical sensor drivers pending
- ⏳ **Real Hardware Testing**: Requires physical components

## Build Configuration

### Environment-Specific Settings
```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_flags = -DLV_CONF_PATH="${PROJECT_DIR}/lv_conf.h" -std=c++14
lib_deps = [LVGL + Arduino libraries]

[env:native]  
platform = native
build_flags = -std=c++17 -Wall -Wextra -g -DLV_CONF_PATH="${PROJECT_DIR}/lv_conf.h"
lib_deps = [LVGL + GoogleTest + ArduinoFake]
```

### Cross-Platform Compilation
- **ESP32**: Arduino framework with hardware-specific implementations
- **Native**: Standard C++ with ArduinoFake for hardware simulation
- **LVGL**: Same UI code compiled for both environments (✅ Verified working)
- **Testing**: Unit tests run on native platform for fast iteration

### Build Status
- **Last Build**: Successful compilation on ESP32
- **Memory Usage**: RAM 21.8% (71,456 bytes), Flash 26.0% (340,761 bytes)
- **Build Time**: ~14 seconds for full compilation
- **Dependencies**: All libraries successfully resolved and linked
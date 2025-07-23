# BayuPandu Project Structure

This document outlines the organization and structure of the BayuPandu project, an open-source paragliding flight computer.

## Repository Organization

The repository is organized into the following main directories:

- **firmware/**: Contains all the ESP32 firmware code
  - **src/**: Source code for the flight computer
  - **.vscode/**: VS Code configuration files
  - **platformio.ini**: PlatformIO configuration file
- **hardware/**: Hardware design files
  - **schematics/**: Circuit diagrams and PCB designs
- **case/**: 3D printable files for the enclosure
- **.kiro/**: Steering files for AI assistance
- **docs/**: Documentation files

## Firmware Structure

The firmware follows a modular architecture with a Hardware Abstraction Layer (HAL) pattern:

```
firmware/
├── platformio.ini    # PlatformIO project configuration
├── lv_conf.h         # LVGL configuration for UI framework
├── src/              # Source code
│   ├── main.cpp      # Main entry point with LVGL initialization
│   ├── config.h      # Configuration variables
│   ├── HAL/          # Hardware Abstraction Layer interfaces & implementations
│   │   ├── I*.h      # Interface definitions (IBarometer, IGPS, IIMU, etc.)
│   │   ├── *Impl.h   # Hardware implementations (MS5611, UbloxGPS, etc.)
│   │   ├── Mock*.h   # Test implementations for ArduinoFake
│   │   └── Arduino*.h # Arduino framework abstraction
│   ├── Data/         # Core data structures (FlightData, GPSData, etc.)
│   ├── Services/     # Business logic services
│   │   ├── VariometerService.*    # Kalman filter & altitude processing
│   │   ├── GPSService.*           # Position tracking & flight detection
│   │   ├── IMUService.*           # Attitude calculation & sensor fusion
│   │   ├── FlightManager.*        # System coordination & state machine
│   │   ├── FlightLogger.*         # IGC format logging
│   │   ├── ConfigService.*        # Configuration management
│   │   └── PowerService.*         # Battery monitoring & power management
│   └── UI/           # User interface components
│       ├── LVGLInit.*             # LVGL framework initialization
│       ├── UserInterface.*       # Main UI controller
│       ├── Screens.*              # Individual screen implementations
│       └── InputManager.*        # Button input handling
├── lib/              # Project-specific libraries
└── test/             # Unit tests with ArduinoFake support
    ├── test_*.cpp    # Unit test implementations
    └── mocks/        # Mock objects for testing
```

## Development Workflow

1. **Feature Development**: Create feature branch
2. **HAL Interface**: Define generic hardware interface (✅ All core interfaces complete)
3. **Simulation**: Implement simulated version for testing (✅ Mock implementations ready)
4. **Unit Tests**: Write tests using simulated components (🚧 ArduinoFake integration complete)
5. **Real Implementation**: Implement actual hardware interface (⏳ In progress)
6. **Hardware Testing**: Test on physical ESP32 with components (⏳ Hardware-dependent)
7. **Integration**: Merge to main branch after validation

### Architecture Principles

- **Layered Architecture**: Clear separation between HAL, Services, and Application layers
- **Dependency Injection**: Services receive HAL interfaces, enabling easy testing and swapping
- **Interface Segregation**: Small, focused interfaces for each hardware component
- **Cross-Platform Support**: Same codebase runs on ESP32 and native testing environment
- **Modern UI Framework**: LVGL provides professional-grade user interface capabilities

## File Naming Conventions

- **Header Files**: Use PascalCase with `.h` extension (e.g., `BarometricSensor.h`)
- **Implementation Files**: Use PascalCase with `.cpp` extension (e.g., `BarometricSensor.cpp`)
- **Interface Files**: Prefix with 'I' (e.g., `IBarometer.h`)
- **Test Files**: Prefix with 'test_' (e.g., `test_barometer.cpp`)

## Code Style Guidelines

- Use C++14 features where appropriate (enabled via build flags)
- Follow Arduino framework conventions for hardware interaction
- Use meaningful variable and function names
- Include comments for complex algorithms and non-obvious code
- Implement proper error handling for hardware failures
- Use const-correctness where applicable
- Prefer composition over inheritance
- Follow RAII principles for resource management
- Use HAL interfaces to maintain testability and modularity

### Testing Strategy

- **Unit Tests**: Core business logic tested with Google Test framework
- **Mock Objects**: Hardware components simulated using ArduinoFake
- **Cross-Platform**: Same tests run on both native and ESP32 environments
- **Integration Tests**: End-to-end testing with recorded flight data
- **Hardware-in-Loop**: Real sensor validation when hardware is available

## LVGL Integration

The project uses LVGL (Light and Versatile Graphics Library) v8.4.0 for modern, responsive UI:

### Configuration
- **Color Depth**: 16-bit (RGB565) for optimal ESP32 performance
- **Memory Allocation**: 48KB dedicated to LVGL operations
- **Widgets Enabled**: Labels, buttons, charts, meters, bars, switches (essential for flight computer UI)
- **Fonts**: Montserrat 12, 14, 16 for readable flight data display
- **Tick Management**: Manual tick handling for cross-platform compatibility

### UI Architecture
- **LVGLInit**: Central initialization and lifecycle management
- **Screen Management**: Individual screens for flight, navigation, settings
- **Input Integration**: Physical buttons mapped to LVGL input events
- **Data Binding**: FlightManager provides real-time data to UI components

### Benefits
- **Professional UI**: Modern widgets and smooth animations
- **Sunlight Readable**: Optimized for outdoor visibility requirements
- **Memory Efficient**: Carefully tuned for ESP32 resource constraints
- **Cross-Platform**: Same UI code works in simulation and on hardware
#ifndef BAYUPANDU_TYPES_H
#define BAYUPANDU_TYPES_H

#include <cstdint>

// A simple 3D vector structure for sensor data.
struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Holds all data fields related to a GPS fix.
struct GPSData {
    double latitude = 0.0;          // degrees
    double longitude = 0.0;         // degrees
    float altitude = 0.0f;           // meters MSL
    float speed = 0.0f;              // m/s
    float heading = 0.0f;            // degrees
    uint8_t satellites = 0;
    float hdop = 99.9f;              // horizontal dilution of precision
    uint32_t timestamp = 0;       // GPS time
    bool hasValidFix = false;
};

// Holds attitude and orientation data from the IMU.
struct AttitudeData {
    float pitch = 0.0f;              // degrees
    float roll = 0.0f;               // degrees
    float yaw = 0.0f;                // degrees (magnetic heading)
    Vector3 acceleration;     // m/s²
    Vector3 angularVelocity;  // rad/s
    bool isCalibrated = false;
};

// A composite structure holding the fused flight data from all sensors.
struct FlightData {
    float altitude = 0.0f;           // meters MSL
    float verticalSpeed = 0.0f;      // m/s
    float pressure = 0.0f;           // hPa
    float temperature = 0.0f;        // °C
    GPSData gpsData;
    AttitudeData attitude;
    uint32_t timestamp = 0;       // milliseconds since boot
    bool isValid = false;
};

// Holds all system configuration settings, loaded from storage.
struct SystemConfig {
    // Variometer settings
    float varioSensitivity = 1.0f;
    float audioVolume = 100.0f;
    float liftThreshold = 0.2f;    // m/s
    float sinkThreshold = -2.5f;   // m/s
    
    // Display settings
    uint8_t brightness = 80;
    uint16_t screenTimeout = 300; // seconds
    
    // GPS settings
    float qnhPressure = 1013.25f; // Sea level pressure for altitude calculation
    
    // Power settings
    float lowBatteryWarning = 3.6f;    // volts
    float criticalBatteryLevel = 3.4f; // volts
    
    // Logging settings
    bool autoStartLogging = true;
    uint16_t loggingInterval = 1; // seconds
};

#endif // BAYUPANDU_TYPES_H

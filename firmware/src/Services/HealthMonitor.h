#pragma once

#include "HAL/IArduino.h"
#include "Services/DataFusionManager.h"

// Monitors sensor health and system status
class HealthMonitor {
public:
    HealthMonitor(DataFusionManager& dataFusion, IArduino& arduino);
    
    // Update health monitoring
    void update();
    
    // Check if all sensors are healthy
    bool areAllSensorsHealthy() const;
    
    // Get error information
    const char* getLastError() const;
    bool hasError() const;
    
    // Set/clear errors
    void setError(const char* error);
    void clearError();

private:
    DataFusionManager& dataFusion;
    IArduino& arduino;
    
    // Error tracking
    char lastError[64];
    uint8_t errorCount;
    uint32_t lastHealthCheck;
    uint32_t lastDataUpdate;
    
    static const uint8_t MAX_ERRORS = 5;
    static const uint32_t HEALTH_CHECK_INTERVAL = 1000; // 1 second
    static const uint32_t DATA_TIMEOUT = 5000; // 5 seconds
    
    void checkSensorHealth();
};

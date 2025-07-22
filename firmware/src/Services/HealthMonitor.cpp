#include "HealthMonitor.h"
#include <string.h>

HealthMonitor::HealthMonitor(DataFusionManager& dataFusion, IArduino& arduino) :
    dataFusion(dataFusion),
    arduino(arduino),
    lastError{0},
    errorCount(0),
    lastHealthCheck(0),
    lastDataUpdate(0)
{
}

void HealthMonitor::update() {
    uint32_t currentTime = arduino.millis();
    
    // Only check health periodically
    if (currentTime - lastHealthCheck < HEALTH_CHECK_INTERVAL) {
        return;
    }
    lastHealthCheck = currentTime;
    
    checkSensorHealth();
}

bool HealthMonitor::areAllSensorsHealthy() const {
    return errorCount == 0;
}

const char* HealthMonitor::getLastError() const {
    return lastError;
}

bool HealthMonitor::hasError() const {
    return lastError[0] != '\0';
}

void HealthMonitor::setError(const char* error) {
    strncpy(lastError, error, sizeof(lastError) - 1);
    lastError[sizeof(lastError) - 1] = '\0';
}

void HealthMonitor::clearError() {
    lastError[0] = '\0';
    errorCount = 0;
}

void HealthMonitor::checkSensorHealth() {
    FlightData data = dataFusion.getFusedFlightData();
    
    // Check if data is being updated
    if (data.timestamp == lastDataUpdate && data.timestamp != 0) {
        // Data hasn't been updated
        if (errorCount < MAX_ERRORS) {
            errorCount++;
            if (errorCount >= MAX_ERRORS) {
                setError("Sensor data stale");
            }
        }
    } else {
        // Data is updating, clear error count
        if (errorCount > 0) {
            errorCount = 0;
            if (hasError() && strstr(lastError, "stale")) {
                clearError(); // Clear stale data error
            }
        }
        lastDataUpdate = data.timestamp;
    }
}

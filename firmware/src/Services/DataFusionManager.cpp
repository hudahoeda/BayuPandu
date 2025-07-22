#include "DataFusionManager.h"

DataFusionManager::DataFusionManager(
    VariometerService& variometerService,
    GPSService& gpsService,
    IMUService& imuService,
    IArduino& arduino
) :
    variometerService(variometerService),
    gpsService(gpsService),
    imuService(imuService),
    arduino(arduino),
    fusedFlightData{}
{
}

void DataFusionManager::fuseData() {
    fuseAltitudeData();
    fusePositionData();
    fuseAttitudeData();
    validateFusedData();
}

FlightData DataFusionManager::getFusedFlightData() const {
    return fusedFlightData;
}

bool DataFusionManager::isDataValid() const {
    return fusedFlightData.isValid;
}

void DataFusionManager::fuseAltitudeData() {
    // Primary altitude from barometric sensor via VariometerService
    fusedFlightData.altitude = gpsService.getGPSData().altitude; // This will be updated when VariometerService provides altitude
    fusedFlightData.verticalSpeed = variometerService.getVerticalSpeed();
    
    // Use GPS altitude as backup or for validation
    GPSData gpsData = gpsService.getGPSData();
    if (gpsData.hasValidFix) {
        // Could implement altitude fusion here
        // For now, prefer barometric altitude for vertical speed calculations
        fusedFlightData.timestamp = arduino.millis();
    }
}

void DataFusionManager::fusePositionData() {
    GPSData gpsData = gpsService.getGPSData();
    fusedFlightData.gpsData = gpsData;
}

void DataFusionManager::fuseAttitudeData() {
    AttitudeData attitudeData = imuService.getAttitudeData();
    fusedFlightData.attitude = attitudeData;
}

void DataFusionManager::validateFusedData() {
    // Basic validation rules
    bool valid = true;
    
    // Check if we have recent data
    uint32_t currentTime = arduino.millis();
    if (currentTime - fusedFlightData.timestamp > 5000) { // 5 seconds old
        valid = false;
    }
    
    // Check for reasonable values
    if (fusedFlightData.altitude < -500.0f || fusedFlightData.altitude > 10000.0f) {
        valid = false; // Reasonable altitude range for paragliding
    }
    
    if (fusedFlightData.verticalSpeed < -30.0f || fusedFlightData.verticalSpeed > 30.0f) {
        valid = false; // Reasonable vertical speed range
    }
    
    fusedFlightData.isValid = valid;
}

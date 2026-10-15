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

void DataFusionManager::setFusedFlightData(const FlightData& data) {
    fusedFlightData = data;
}

FlightData DataFusionManager::getFusedFlightData() const {
    return fusedFlightData;
}

bool DataFusionManager::isDataValid() const {
    return fusedFlightData.isValid;
}

void DataFusionManager::fuseAltitudeData() {
    // Altitude and climb come from the barometer only. Without it the data goes
    // stale and the display shows unknown, rather than a GPS-derived 0 m/s that
    // would read as level flight.
    if (variometerService.hasData()) {
        fusedFlightData.altitude = variometerService.getAltitude();
        fusedFlightData.verticalSpeed = variometerService.getVerticalSpeed();
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

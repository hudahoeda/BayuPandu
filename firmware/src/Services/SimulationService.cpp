#include "SimulationService.h" // This should now correctly include ../Data/Types.h
#include "../Data/Types.h" // Explicitly include Data/Types.h for GPSData definition
#include <cmath> // For round, fmod, etc.
#include <algorithm> // For std::min, std::max

SimulationService::SimulationService(IArduino& arduino)
    : arduino(arduino),
      igcParser(),
      currentRecordIndex(0),
      simulationStartTime(0),
      igcFileStartTime(0),
      simulatedBaroAltitude(0.0f),
      simulatedVerticalSpeed(0.0f),
      simulationActive(false)
{
    simulatedGPSData = {}; // Initialize struct
    simulatedAttitude = {}; // Initialize struct
}

bool SimulationService::initialize(const std::string& igcFilePath) {
    simulationActive = igcParser.loadFromFile(igcFilePath);
    if (simulationActive) {
        currentRecordIndex = 0;
        simulationStartTime = arduino.millis();
        igcFileStartTime = igcParser.getRecord(0).timestamp;
        // Initialize with first record's data
        IGCRecord firstRecord = igcParser.getRecord(0);
        simulatedGPSData.latitude = firstRecord.latitude;
        simulatedGPSData.longitude = firstRecord.longitude;
        simulatedGPSData.altitude = firstRecord.gpsAltitude;
        simulatedGPSData.hasValidFix = true; // Assume valid fix for simulation
        simulatedBaroAltitude = firstRecord.baroAltitude;
        simulatedGPSData.speed = firstRecord.speed;
        simulatedGPSData.heading = firstRecord.heading;
        simulatedGPSData.satellites = firstRecord.numSatellites; // Use 'satellites' as per GPSData struct
        simulatedGPSData.hdop = firstRecord.hdop;
        simulatedGPSData.timestamp = firstRecord.timestamp;
    }
    return simulationActive;
}

void SimulationService::update() {
    if (!simulationActive || igcParser.getRecordCount() < 2) {
        return;
    }

    uint32_t currentSimTime = arduino.millis() - simulationStartTime;
    // Map currentSimTime to IGC file time (relative to igcFileStartTime)
    uint32_t targetIGCTime = igcFileStartTime + currentSimTime;

    // Advance record index if current record's timestamp is less than targetIGCTime
    while (currentRecordIndex < igcParser.getRecordCount() - 1) {
        if (igcParser.getRecord(currentRecordIndex + 1).timestamp <= targetIGCTime) {
            currentRecordIndex++;
        } else {
            break;
        }
    }

    // If we've reached the end of the file, stop simulation
    if (currentRecordIndex >= igcParser.getRecordCount() - 1) {
        simulationActive = false;
        return;
    }

    // Interpolate data between currentRecord and nextRecord
    IGCRecord record1 = igcParser.getRecord(currentRecordIndex);
    IGCRecord record2 = igcParser.getRecord(currentRecordIndex + 1);

    interpolateData(record1, record2, targetIGCTime);
}

GPSData SimulationService::getGPSData() const {
    return simulatedGPSData;
}

float SimulationService::getBarometricAltitude() const {
    return simulatedBaroAltitude;
}

float SimulationService::getVerticalSpeed() const {
    return simulatedVerticalSpeed;
}

AttitudeData SimulationService::getAttitudeData() const {
    return simulatedAttitude; // Currently returns default, needs actual simulation
}

bool SimulationService::isActive() const {
    return simulationActive;
}

void SimulationService::interpolateData(const IGCRecord& record1, const IGCRecord& record2, uint32_t currentTimeMillis) {
    // Calculate interpolation factor (0.0 to 1.0)
    float factor = 0.0f;
    uint32_t timeDiff = record2.timestamp - record1.timestamp;
    if (timeDiff > 0) {
        factor = (float)(currentTimeMillis - record1.timestamp) / timeDiff;
    }

    // Clamp factor to ensure it's within [0, 1]
    factor = std::max(0.0f, std::min(1.0f, factor));

    // Interpolate latitude and longitude
    simulatedGPSData.latitude = record1.latitude + (record2.latitude - record1.latitude) * factor;
    simulatedGPSData.longitude = record1.longitude + (record2.longitude - record1.longitude) * factor;

    // Interpolate GPS altitude
    float interpolatedGpsAltitude = record1.gpsAltitude + (record2.gpsAltitude - record1.gpsAltitude) * factor;
    
    // Interpolate barometric altitude
    float interpolatedBaroAltitude = record1.baroAltitude + (record2.baroAltitude - record1.baroAltitude) * factor;
    
    // Calculate vertical speed based on interpolated barometric altitude
    // This is a simplified calculation and might need to be refined for accuracy
    if (simulatedBaroAltitude != 0.0f) { // Avoid division by zero on first run
        simulatedVerticalSpeed = calculateVerticalSpeed(interpolatedBaroAltitude, simulatedBaroAltitude, (currentTimeMillis - simulatedGPSData.timestamp));
    } else {
        simulatedVerticalSpeed = 0.0f;
    }
    
    simulatedBaroAltitude = interpolatedBaroAltitude;
    simulatedGPSData.altitude = interpolatedGpsAltitude;

    // Update GPS data fields
    simulatedGPSData.speed = record1.speed + (record2.speed - record1.speed) * factor;
    simulatedGPSData.heading = record1.heading + (record2.heading - record1.heading) * factor;
    simulatedGPSData.satellites = record1.numSatellites; // Use 'satellites' as per GPSData struct
    simulatedGPSData.hdop = record1.hdop;                   // Simplistic: just take from record1
    simulatedGPSData.timestamp = currentTimeMillis;
}

float SimulationService::calculateVerticalSpeed(float currentAltitude, float previousAltitude, uint32_t deltaTimeMillis) {
    if (deltaTimeMillis == 0) return 0.0f;
    float deltaTimeSeconds = (float)deltaTimeMillis / 1000.0f;
    return (currentAltitude - previousAltitude) / deltaTimeSeconds;
}

#pragma once

#include "../Data/Types.h"
#include "IGCParser.h" // IGCParser is in the same directory
#include "../HAL/IArduino.h" // IArduino is in HAL directory

class SimulationService {
public:
    SimulationService(IArduino& arduino);

    // Initializes the simulation with an IGC file.
    bool initialize(const std::string& igcFilePath);

    // Returns simulated GPS data.
    GPSData getGPSData() const;

    // Returns simulated barometric altitude.
    float getBarometricAltitude() const;

    // Returns simulated vertical speed.
    float getVerticalSpeed() const;

    // Returns simulated IMU data.
    AttitudeData getAttitudeData() const;

    // Updates the simulation state (moves to the next record based on time).
    void update();

    // Checks if the simulation is active.
    bool isActive() const;

private:
    IArduino& arduino;
    IGCParser igcParser;
    size_t currentRecordIndex;
    uint32_t simulationStartTime; // millis() when simulation started
    uint32_t igcFileStartTime;    // timestamp of first IGC record

    GPSData simulatedGPSData;
    float simulatedBaroAltitude;
    float simulatedVerticalSpeed;
    AttitudeData simulatedAttitude; // For future IMU simulation

    bool simulationActive;

    // Interpolates data between two IGC records based on current simulation time.
    void interpolateData(const IGCRecord& record1, const IGCRecord& record2, uint32_t currentTimeMillis);
    
    // Calculates vertical speed from altitude changes.
    float calculateVerticalSpeed(float currentAltitude, float previousAltitude, uint32_t deltaTimeMillis);
};

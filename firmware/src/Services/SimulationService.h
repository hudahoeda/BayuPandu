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

    // Barometric altitude, smooth between fixes: what a barometer would have
    // measured, before its own noise (ReplayBarometer adds that).
    float getBarometricAltitude() const;

    // The log's own climb rate, for comparing with what the vario filter makes of it.
    float getVerticalSpeed() const;

    // Returns simulated IMU data.
    AttitudeData getAttitudeData() const;

    // Updates the simulation state (moves to the next record based on time).
    void update();

    // Checks if the simulation is active.
    bool isActive() const;

    // Replay-only fault injection, so the warning states can be seen without
    // hardware: a lost GPS fix, and a barometer that stopped answering.
    void setGpsLost(bool lost) { gpsLost = lost; }
    void setBaroLost(bool lost) { baroLost = lost; }
    // A GPS module that stopped talking altogether (implies no fix).
    void setGpsSilent(bool silent) { gpsSilent = silent; }
    bool isGpsSilent() const { return gpsSilent; }
    bool isGpsLost() const { return gpsLost; }
    bool isBaroLost() const { return baroLost; }
    bool hasBaroData() const { return simulationActive && !baroLost; }

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
    bool gpsLost = false;
    bool baroLost = false;
    bool gpsSilent = false;

    // Interpolates data between two IGC records based on current simulation time.
    void interpolateData(const IGCRecord& record1, const IGCRecord& record2, uint32_t currentTimeMillis);
};

#pragma once

#include "Data/Types.h"
#include <cstdint>

// Derived, pilot-facing numbers that need history: averaged climb, glide ratio,
// time airborne and the way back to takeoff.
struct FlightStatsSnapshot {
    float averageClimb = 0.0f;   // m/s over the averaging window
    float averageSpeed = 0.0f;   // m/s ground speed over the window
    float glideRatio = 0.0f;     // distance per height lost; valid only when hasGlideRatio
    bool hasGlideRatio = false;
    bool airborne = false;
    uint32_t airborneSeconds = 0;
    // Current thermal, or the last one once the climb has stopped.
    bool inThermal = false;
    bool hasThermal = false;
    float thermalAverage = 0.0f;     // m/s, height gained / time in thermal
    float thermalGain = 0.0f;        // m
    uint32_t thermalSeconds = 0;
    bool hasTakeoff = false;
    float takeoffDistanceM = 0.0f;
    float takeoffBearingDeg = 0.0f;  // true bearing from the pilot to takeoff
};

class FlightStats {
public:
    void update(const FlightData& data, uint32_t nowMs);
    void reset();
    const FlightStatsSnapshot& get() const { return snapshot; }

private:
    static constexpr int kWindow = 20;              // samples, one per second
    static constexpr uint32_t kSamplePeriodMs = 1000;
    static constexpr float kTakeoffSpeed = 4.0f;    // m/s ground speed that counts as flying
    static constexpr float kMinSinkForGlide = 0.3f; // m/s; below this L/D is meaningless

    float climbSamples[kWindow] = {};
    float speedSamples[kWindow] = {};
    int sampleCount = 0;
    int nextSample = 0;
    uint32_t lastSampleMs = 0;
    uint32_t takeoffMs = 0;
    uint32_t thermalStartMs = 0;
    float thermalStartAlt = 0.0f;
    static constexpr float kThermalEnter = 0.3f;  // m/s averaged climb to call it a thermal
    static constexpr float kThermalExit = 0.0f;
    double takeoffLat = 0.0, takeoffLon = 0.0;
    FlightStatsSnapshot snapshot;
};

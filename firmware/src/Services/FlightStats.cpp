#include "FlightStats.h"
#include "Geo.h"

void FlightStats::reset() {
    *this = FlightStats();
}

void FlightStats::update(const FlightData& data, uint32_t nowMs) {
    const GPSData& gps = data.gpsData;

    // First valid fix is where the flight starts from.
    if (gps.hasValidFix && !snapshot.hasTakeoff) {
        snapshot.hasTakeoff = true;
        takeoffLat = gps.latitude;
        takeoffLon = gps.longitude;
    }
    if (gps.hasValidFix && snapshot.hasTakeoff) {
        snapshot.takeoffDistanceM = geo::distanceM(gps.latitude, gps.longitude, takeoffLat, takeoffLon);
        snapshot.takeoffBearingDeg = geo::bearingDeg(gps.latitude, gps.longitude, takeoffLat, takeoffLon);
    }

    if (!snapshot.airborne && gps.hasValidFix && gps.speed > kTakeoffSpeed) {
        snapshot.airborne = true;
        takeoffMs = nowMs;
    }
    if (snapshot.airborne) {
        snapshot.airborneSeconds = (nowMs - takeoffMs) / 1000;
    }

    if (!data.isValid || nowMs - lastSampleMs < kSamplePeriodMs) {
        return;
    }
    lastSampleMs = nowMs;
    climbSamples[nextSample] = data.verticalSpeed;
    speedSamples[nextSample] = gps.speed;
    nextSample = (nextSample + 1) % kWindow;
    if (sampleCount < kWindow) sampleCount++;

    float climbSum = 0.0f, speedSum = 0.0f;
    for (int i = 0; i < sampleCount; i++) {
        climbSum += climbSamples[i];
        speedSum += speedSamples[i];
    }
    snapshot.averageClimb = climbSum / sampleCount;
    snapshot.averageSpeed = speedSum / sampleCount;
    snapshot.hasGlideRatio = snapshot.averageClimb < -kMinSinkForGlide && snapshot.averageSpeed > 1.0f;
    snapshot.glideRatio = snapshot.hasGlideRatio ? snapshot.averageSpeed / -snapshot.averageClimb : 0.0f;

    // Thermal: entered when the averaged climb passes kThermalEnter, left when it drops
    // to kThermalExit. Entry is back-dated by the averaging window it took to notice.
    if (!snapshot.inThermal && snapshot.averageClimb > kThermalEnter && sampleCount >= 5) {
        snapshot.inThermal = true;
        snapshot.hasThermal = true;
        uint32_t lookback = sampleCount * kSamplePeriodMs;
        thermalStartMs = nowMs > lookback ? nowMs - lookback : 0;
        thermalStartAlt = data.altitude - snapshot.averageClimb * sampleCount;
    } else if (snapshot.inThermal && snapshot.averageClimb <= kThermalExit) {
        snapshot.inThermal = false;
    }
    if (snapshot.inThermal) {
        snapshot.thermalSeconds = (nowMs - thermalStartMs) / 1000;
        snapshot.thermalGain = data.altitude - thermalStartAlt;
        snapshot.thermalAverage = snapshot.thermalSeconds > 0
            ? snapshot.thermalGain / snapshot.thermalSeconds : 0.0f;
    }
}

#pragma once

#include "HAL/IBarometer.h"
#include "Services/SimulationService.h"
#include <cmath>
#include <cstdint>

// A barometer that reads the replayed flight, so a replay goes through the same
// VariometerService and Kalman filter as a real MS5611. The replay's smoothed
// altitude gets Gaussian noise like a real sensor's before it becomes pressure;
// without it the filter would look calmer in replay than it will in the air.
class ReplayBarometer : public IBarometer {
public:
    // MS5611 at full oversampling, as the filter's own tuning assumes (m, 1 sigma).
    static constexpr float kNoiseM = 0.2f;

    explicit ReplayBarometer(const SimulationService& sim) : sim(sim) {}

    bool initialize() override { return true; }

    bool readPressure(float& pressure) override {
        if (!sim.hasBaroData()) return false;
        pressure = pressureAt(sim.getBarometricAltitude() + kNoiseM * gaussian());
        return true;
    }

    bool readTemperature(float& temperature) override {
        temperature = 15.0f;
        return true;
    }

    // International standard atmosphere, the inverse of pressureAt().
    float calculateAltitude(float pressure, float seaLevelPressure = 1013.25f) override {
        return 44330.0f * (1.0f - std::pow(pressure / seaLevelPressure, 1.0f / 5.255f));
    }

    bool isHealthy() override { return sim.hasBaroData(); }

    static float pressureAt(float altitude, float seaLevelPressure = 1013.25f) {
        return seaLevelPressure * std::pow(1.0f - altitude / 44330.0f, 5.255f);
    }

private:
    // Box-Muller over a fixed-seed xorshift, so headless runs stay repeatable.
    float gaussian() {
        if (hasSpare) {
            hasSpare = false;
            return spare;
        }
        float u1 = uniform(), u2 = uniform();
        float r = std::sqrt(-2.0f * std::log(u1));
        spare = r * std::sin(6.2831853f * u2);
        hasSpare = true;
        return r * std::cos(6.2831853f * u2);
    }
    float uniform() {  // (0, 1]
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return ((state >> 8) + 1) / 16777216.0f;
    }

    const SimulationService& sim;
    uint32_t state = 0x2545F491u;
    float spare = 0.0f;
    bool hasSpare = false;
};

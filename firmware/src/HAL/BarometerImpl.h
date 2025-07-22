#pragma once

#include "IBarometer.h"

class BarometerImpl : public IBarometer {
public:
    bool initialize() override { return true; }
    bool readPressure(float& pressure) override { pressure = 1013.25f; return true; }
    bool readTemperature(float& temperature) override { temperature = 25.0f; return true; }
    float calculateAltitude(float pressure, float seaLevelPressure = 1013.25f) override { return 0.0f; }
    bool isHealthy() override { return true; }
};

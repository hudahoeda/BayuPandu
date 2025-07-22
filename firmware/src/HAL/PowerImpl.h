#pragma once

#include "IPower.h"

class PowerImpl : public IPower {
public:
    bool initialize() override { return true; }
    float getBatteryVoltage() override { return 4.2f; }
    uint8_t getBatteryPercentage() override { return 100; }
    bool isCharging() override { return false; }
    void enterLowPowerMode() override {}
};

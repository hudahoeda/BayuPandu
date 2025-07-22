#pragma once

#include "IGPS.h"

class GPSImpl : public IGPS {
public:
    bool initialize() override { return true; }
    bool update() override { return true; }
    bool hasValidFix() override { return false; }
    GPSData getCurrentPosition() override { return GPSData{}; }
    uint8_t getSatelliteCount() override { return 0; }
    float getHDOP() override { return 99.9f; }
};

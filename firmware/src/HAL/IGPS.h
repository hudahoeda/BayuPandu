#ifndef IGPS_H
#define IGPS_H

#include "Data/Types.h"

class IGPS {
public:
    virtual ~IGPS() = default;

    virtual bool initialize() = 0;
    virtual bool update() = 0;
    virtual bool hasValidFix() = 0;
    virtual GPSData getCurrentPosition() = 0;
    virtual uint8_t getSatelliteCount() = 0;
    virtual float getHDOP() = 0;
};

#endif // IGPS_H

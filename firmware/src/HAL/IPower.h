#ifndef IPOWER_H
#define IPOWER_H

#include <cstdint>

class IPower {
public:
    virtual ~IPower() = default;

    virtual bool initialize() = 0;
    virtual float getBatteryVoltage() = 0;
    virtual uint8_t getBatteryPercentage() = 0;
    virtual bool isCharging() = 0;
    virtual void enterLowPowerMode() = 0;
};

#endif // IPOWER_H

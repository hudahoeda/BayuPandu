#pragma once

#include "HAL/IPower.h"
#include "HAL/IAudio.h"
#include "HAL/IArduino.h"
#include "Services/ConfigService.h"

class PowerService
{
public:
    PowerService(IPower& power, IAudio& audio, ConfigService& configService, IArduino& arduino);

    void update();
    float getBatteryVoltage() const;
    bool isLowBattery() const;
    bool isCriticalBattery() const;

private:
    IPower& power;
    IAudio& audio;
    ConfigService& configService;
    IArduino& arduino;
    float batteryVoltage;
};

#pragma once

#include "HAL/IPower.h"
#include "HAL/IAudio.h"
#include "HAL/IArduino.h"
#include "Services/ConfigService.h"
#include "Data/Types.h" // Include BatteryInfo definition

class PowerService
{
public:
    PowerService(IPower& power, IAudio& audio, ConfigService& configService, IArduino& arduino);

    void update();
    BatteryInfo getBatteryInfo() const;
    float getBatteryVoltage() const; // Keep for backward compatibility if needed
    bool isLowBattery() const;
    bool isCriticalBattery() const;

private:
    IPower& power;
    IAudio& audio;
    ConfigService& configService;
    IArduino& arduino;
    float batteryVoltage;
};

#include "PowerService.h"
#include "Data/Types.h" // For BatteryInfo

PowerService::PowerService(IPower& power, IAudio& audio, ConfigService& configService, IArduino& arduino)
    : power(power),
      audio(audio),
      configService(configService),
      arduino(arduino),
      batteryVoltage(0.0f)
{
}

void PowerService::update()
{
    batteryVoltage = power.getBatteryVoltage();

    if (isCriticalBattery())
    {
        // Trigger critical battery alert (e.g., continuous tone)
        audio.playAlert(IAudio::AlertType::Error); // Using Error as a stand-in for critical
    }
    else if (isLowBattery())
    {
        // Trigger low battery warning (e.g., periodic beep)
        static unsigned long lastBeep = 0;
        if (arduino.millis() - lastBeep > 5000)
        {
            audio.playAlert(IAudio::AlertType::LowBattery);
            lastBeep = arduino.millis();
        }
    }
}

BatteryInfo PowerService::getBatteryInfo() const
{
    BatteryInfo info;
    info.voltage = batteryVoltage;
    // Simple linear mapping for percentage
    // Assuming 4.2V is 100% and criticalBatteryLevel is 0%
    float maxVoltage = 4.2f; 
    float minVoltage = configService.getConfig().criticalBatteryLevel;
    
    if (batteryVoltage >= maxVoltage) {
        info.percentage = 100;
    } else if (batteryVoltage <= minVoltage) {
        info.percentage = 0;
    } else {
        info.percentage = (int)(((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0f);
    }
    
    info.isCharging = power.isCharging(); // Assuming IPower has isCharging()
    return info;
}

float PowerService::getBatteryVoltage() const
{
    return batteryVoltage;
}

bool PowerService::isLowBattery() const
{
    return batteryVoltage < configService.getConfig().lowBatteryWarning;
}

bool PowerService::isCriticalBattery() const
{
    return batteryVoltage < configService.getConfig().criticalBatteryLevel;
}

#include "PowerService.h"

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

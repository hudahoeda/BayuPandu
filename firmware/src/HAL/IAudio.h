#ifndef IAUDIO_H
#define IAUDIO_H

#include <cstdint>

class IAudio {
public:
    enum class AlertType {
        Startup,
        Shutdown,
        GPSFix,
        Error,
        LowBattery
    };

    virtual ~IAudio() = default;

    virtual bool initialize() = 0;
    virtual void tone(unsigned int frequency, unsigned long duration = 0) = 0;
    virtual void noTone() = 0;
    virtual void setVolume(uint8_t percentage) = 0;
    virtual void playAlert(AlertType type) = 0;
};

#endif // IAUDIO_H

#pragma once

#include "HAL/IBarometer.h"
#include "HAL/IAudio.h"
#include "HAL/IArduino.h"
#include "Services/VarioFilter.h"

class VariometerService
{
public:
    VariometerService(IBarometer& barometer, IAudio& audio, IArduino& arduino);

    void update();

    float getVerticalSpeed() const;
    float getAltitude() const;
    // True once the filter has a reading and the barometer has not failed since.
    bool hasData() const;

    // Climb tone above this rate, sink alarm below the other (m/s).
    static constexpr float kClimbToneThreshold = 0.2f;
    static constexpr float kSinkAlarmThreshold = -2.5f;

private:
    void updateAudio();

    IBarometer& barometer;
    IAudio& audio;
    IArduino& arduino;

    VarioFilter filter;
    uint32_t lastSampleMs;
    bool lastReadOk;
};

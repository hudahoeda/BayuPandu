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
    // Silences the climb and sink tones, e.g. while the startup chime plays.
    void setMuted(bool muted);
    // Reads `replay` instead of the hardware barometer, or the hardware again with
    // nullptr. Either way the filter restarts, so no climb carries across.
    void setReplaySource(IBarometer* replay);

    // Climb beeps above this rate, sink alarm below the other (m/s).
    static constexpr float kClimbToneThreshold = 0.2f;
    static constexpr float kSinkAlarmThreshold = -2.5f;

    // The classic XC sound. In lift: beeps that rise in pitch and come faster as
    // the climb grows, about 2 a second at 0.5 m/s and 8 at 5 m/s, sounding half
    // of each cycle. In strong sink: a steady low tone that falls with the sink.
    static unsigned int climbPitchHz(float vs);
    static uint32_t beepPeriodMs(float vs);
    static unsigned int sinkPitchHz(float vs);

private:
    void updateAudio();
    void silence();

    IBarometer& hardware;
    IBarometer* barometer;  // hardware, or a replay
    IAudio& audio;
    IArduino& arduino;

    VarioFilter filter;
    uint32_t lastSampleMs;
    bool lastReadOk;
    bool muted = false;

    enum class Sound { Silent, Climb, Sink };
    Sound sound = Sound::Silent;
    uint32_t nextBeepMs = 0;    // when the next climb beep starts
    unsigned int sinkHz = 0;    // pitch of the sink tone now sounding
};

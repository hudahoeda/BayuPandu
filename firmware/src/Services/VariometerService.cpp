#include "VariometerService.h"

VariometerService::VariometerService(IBarometer& barometer, IAudio& audio, IArduino& arduino)
    : hardware(barometer),
      barometer(&barometer),
      audio(audio),
      arduino(arduino),
      filter(),
      lastSampleMs(0),
      lastReadOk(false)
{
}

void VariometerService::update()
{
    float pressure;
    if (!barometer->readPressure(pressure))
    {
        // Keep the last estimate but stop claiming it is live; the filter
        // restarts on the next good reading if the gap grew too long.
        lastReadOk = false;
        silence();
        return;
    }

    uint32_t now = arduino.millis();
    float dt = filter.isInitialized() ? (now - lastSampleMs) / 1000.0f : 0.0f;
    lastSampleMs = now;

    filter.update(barometer->calculateAltitude(pressure), dt);
    lastReadOk = true;
    updateAudio();
}

void VariometerService::setReplaySource(IBarometer* replay)
{
    barometer = replay ? replay : &hardware;
    filter.reset();
    lastReadOk = false;
    silence();
}

void VariometerService::setMuted(bool value)
{
    if (value) silence();
    muted = value;
}

void VariometerService::silence()
{
    if (!muted && sound != Sound::Silent) audio.noTone();
    sound = Sound::Silent;
}

unsigned int VariometerService::climbPitchHz(float vs)
{
    float hz = 700.0f + vs * 150.0f;
    return static_cast<unsigned int>(hz > 2000.0f ? 2000.0f : hz);
}

uint32_t VariometerService::beepPeriodMs(float vs)
{
    float perSecond = 1.4f + 1.3f * vs;
    if (perSecond > 10.0f) perSecond = 10.0f;
    return static_cast<uint32_t>(1000.0f / perSecond);
}

unsigned int VariometerService::sinkPitchHz(float vs)
{
    float hz = 400.0f + vs * 20.0f;
    return static_cast<unsigned int>(hz < 200.0f ? 200.0f : hz);
}

void VariometerService::updateAudio()
{
    if (muted) return;
    float vs = filter.verticalSpeed();
    uint32_t now = arduino.millis();
    if (vs >= kClimbToneThreshold)
    {
        // Each beep takes the pitch and pace of the climb when it starts, and
        // stops by itself; the first one starts as soon as the lift is found.
        // The next is due a period after this one was due, not after this
        // update, so the pace holds even though updates come only every ~20-50 ms.
        bool starting = sound != Sound::Climb;
        if (starting || static_cast<int32_t>(now - nextBeepMs) >= 0)
        {
            uint32_t period = beepPeriodMs(vs);
            audio.tone(climbPitchHz(vs), period / 2);
            bool behind = starting || static_cast<int32_t>(now - nextBeepMs) >= static_cast<int32_t>(period);
            nextBeepMs = (behind ? now : nextBeepMs) + period;
        }
        sound = Sound::Climb;
    }
    else if (vs <= kSinkAlarmThreshold)
    {
        // Steady until it ends; retuned only when the pitch moves audibly.
        unsigned int hz = sinkPitchHz(vs);
        if (sound != Sound::Sink || hz + 5 < sinkHz || hz > sinkHz + 5)
        {
            audio.tone(hz, 0);
            sinkHz = hz;
        }
        sound = Sound::Sink;
    }
    else
    {
        silence();
    }
}

float VariometerService::getVerticalSpeed() const
{
    return filter.verticalSpeed();
}

float VariometerService::getAltitude() const
{
    return filter.altitude();
}

bool VariometerService::hasData() const
{
    return lastReadOk && filter.isInitialized();
}

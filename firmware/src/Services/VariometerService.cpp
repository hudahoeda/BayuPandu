#include "VariometerService.h"

VariometerService::VariometerService(IBarometer& barometer, IAudio& audio, IArduino& arduino)
    : barometer(barometer),
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
    if (!barometer.readPressure(pressure))
    {
        // Keep the last estimate but stop claiming it is live; the filter
        // restarts on the next good reading if the gap grew too long.
        lastReadOk = false;
        if (!muted) audio.noTone();
        return;
    }

    uint32_t now = arduino.millis();
    float dt = filter.isInitialized() ? (now - lastSampleMs) / 1000.0f : 0.0f;
    lastSampleMs = now;

    filter.update(barometer.calculateAltitude(pressure), dt);
    lastReadOk = true;
    updateAudio();
}

void VariometerService::setMuted(bool value)
{
    if (value && !muted) audio.noTone();
    muted = value;
}

void VariometerService::updateAudio()
{
    if (muted) return;
    float vs = filter.verticalSpeed();
    if (vs >= kClimbToneThreshold)
    {
        float hz = 700.0f + vs * 150.0f;
        audio.tone(static_cast<unsigned int>(hz > 2000.0f ? 2000.0f : hz), 100);
    }
    else if (vs <= kSinkAlarmThreshold)
    {
        float hz = 400.0f + vs * 20.0f;
        audio.tone(static_cast<unsigned int>(hz < 200.0f ? 200.0f : hz), 100);
    }
    else
    {
        audio.noTone();
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

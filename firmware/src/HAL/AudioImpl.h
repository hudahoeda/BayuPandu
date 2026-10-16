#pragma once

#include "IAudio.h"
#include "config.h"
#include <Arduino.h>

// Passive piezo buzzer on PIN_BUZZER, driven with the Arduino core's tone(), which
// on the ESP32 runs on an LEDC channel and stops by itself after `duration` ms.
class AudioImpl : public IAudio {
public:
    bool initialize() override {
        pinMode(PIN_BUZZER, OUTPUT);
        return true;
    }
    void tone(unsigned int frequency, unsigned long duration = 0) override {
        if (volume == 0 || frequency == 0) return;
        ::tone(PIN_BUZZER, frequency, duration);
    }
    void noTone() override {
        ::noTone(PIN_BUZZER);
    }
    // A piezo's loudness can't be set without extra hardware; 0 mutes it.
    void setVolume(uint8_t percentage) override {
        volume = percentage;
        if (volume == 0) noTone();
    }
    void playAlert(AlertType type) override {}

private:
    uint8_t volume = 100;
};

#pragma once

#include "IAudio.h"
#include "config.h"
#include <Arduino.h>

// On the ESP32-C3, GPIO 12-17 belong to the SPI flash; driving one can hang the
// chip. Keep the buzzer off rather than risk that until config.h's pin map moves it.
#if defined(CONFIG_IDF_TARGET_ESP32C3) && PIN_BUZZER >= 12 && PIN_BUZZER <= 17
#warning "PIN_BUZZER is an SPI flash pin on the ESP32-C3: buzzer disabled until the pin map is fixed"
#define BAYUPANDU_BUZZER_ENABLED 0
#else
#define BAYUPANDU_BUZZER_ENABLED 1
#endif

// Passive piezo buzzer on PIN_BUZZER, driven with the Arduino core's tone(), which
// on the ESP32 runs on an LEDC channel and stops by itself after `duration` ms.
class AudioImpl : public IAudio {
public:
    bool initialize() override {
        if (!BAYUPANDU_BUZZER_ENABLED) return false;
        pinMode(PIN_BUZZER, OUTPUT);
        return true;
    }
    void tone(unsigned int frequency, unsigned long duration = 0) override {
        if (!BAYUPANDU_BUZZER_ENABLED || volume == 0 || frequency == 0) return;
        ::tone(PIN_BUZZER, frequency, duration);
    }
    void noTone() override {
        if (BAYUPANDU_BUZZER_ENABLED) ::noTone(PIN_BUZZER);
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

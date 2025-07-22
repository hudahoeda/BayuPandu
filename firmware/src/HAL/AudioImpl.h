#pragma once

#include "IAudio.h"

class AudioImpl : public IAudio {
public:
    bool initialize() override { return true; }
    void tone(unsigned int frequency, unsigned long duration = 0) override {}
    void noTone() override {}
    void setVolume(uint8_t percentage) override {}
    void playAlert(AlertType type) override {}
};

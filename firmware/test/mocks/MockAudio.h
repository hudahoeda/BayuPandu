#ifndef MOCK_AUDIO_H
#define MOCK_AUDIO_H

#include "HAL/IAudio.h"
#include <vector>
#include <utility>

class MockAudio : public IAudio {
public:
    struct ToneEvent {
        unsigned int frequency;
        unsigned long duration;
    };

    // --- IAudio Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    void tone(unsigned int frequency, unsigned long duration = 0) override {
        tone_call_count_++;
        last_tone_ = {frequency, duration};
        tone_history_.push_back(last_tone_);
        is_tone_active_ = true;
    }

    void noTone() override {
        no_tone_call_count_++;
        is_tone_active_ = false;
    }

    void setVolume(uint8_t percentage) override {
        volume_ = percentage;
    }

    void playAlert(AlertType type) override {
        play_alert_call_count_++;
        last_alert_type_ = type;
        alert_history_.push_back(type);
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getToneCallCount() const { return tone_call_count_; }
    int getNoToneCallCount() const { return no_tone_call_count_; }
    int getPlayAlertCallCount() const { return play_alert_call_count_; }
    uint8_t getCurrentVolume() const { return volume_; }
    bool isToneActive() const { return is_tone_active_; }
    ToneEvent getLastTone() const { return last_tone_; }
    AlertType getLastAlertType() const { return last_alert_type_; }
    const std::vector<ToneEvent>& getToneHistory() const { return tone_history_; }
    const std::vector<AlertType>& getAlertHistory() const { return alert_history_; }

    void reset() {
        initialize_called_ = false;
        tone_call_count_ = 0;
        no_tone_call_count_ = 0;
        play_alert_call_count_ = 0;
        volume_ = 100;
        is_tone_active_ = false;
        last_tone_ = {0, 0};
        last_alert_type_ = AlertType::Startup;
        tone_history_.clear();
        alert_history_.clear();
    }

private:
    bool initialize_called_ = false;
    int tone_call_count_ = 0;
    int no_tone_call_count_ = 0;
    int play_alert_call_count_ = 0;
    uint8_t volume_ = 100;
    bool is_tone_active_ = false;
    ToneEvent last_tone_{0, 0};
    AlertType last_alert_type_ = AlertType::Startup;
    std::vector<ToneEvent> tone_history_;
    std::vector<AlertType> alert_history_;
};

#endif // MOCK_AUDIO_H

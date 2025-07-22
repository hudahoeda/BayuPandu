#ifndef MOCK_DISPLAY_H
#define MOCK_DISPLAY_H

#include "HAL/IDisplay.h"
#include <vector>

class MockDisplay : public IDisplay {
public:
    // --- Test Control Methods ---
    void setBrightness(uint8_t percentage) override {
        brightness_ = percentage;
    }

    // --- IDisplay Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    void clear() override {
        clear_call_count_++;
        last_drawn_data_ = FlightData{}; // Reset data on clear
    }

    void drawUI(const FlightData& data) override {
        drawUI_call_count_++;
        last_drawn_data_ = data;
        draw_history_.push_back(data);
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getClearCallCount() const { return clear_call_count_; }
    int getDrawUICallCount() const { return drawUI_call_count_; }
    uint8_t getCurrentBrightness() const { return brightness_; }
    FlightData getLastDrawnData() const { return last_drawn_data_; }
    const std::vector<FlightData>& getDrawHistory() const { return draw_history_; }
    void reset() {
        initialize_called_ = false;
        clear_call_count_ = 0;
        drawUI_call_count_ = 0;
        brightness_ = 100;
        last_drawn_data_ = FlightData{};
        draw_history_.clear();
    }

private:
    bool initialize_called_ = false;
    int clear_call_count_ = 0;
    int drawUI_call_count_ = 0;
    uint8_t brightness_ = 100;
    FlightData last_drawn_data_{};
    std::vector<FlightData> draw_history_;
};

#endif // MOCK_DISPLAY_H

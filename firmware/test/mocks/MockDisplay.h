#ifndef MOCK_DISPLAY_H
#define MOCK_DISPLAY_H

#include "HAL/IDisplay.h"
#include <vector>
#include <string>

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
        display_buffer_.clear();
    }

    void drawUI(const FlightData& data) override {
        drawUI_call_count_++;
        last_drawn_data_ = data;
        draw_history_.push_back(data);
    }

    // Text operations
    void setTextSize(uint8_t size) override {
        current_text_size_ = size;
    }

    void setCursor(int16_t x, int16_t y) override {
        cursor_x_ = x;
        cursor_y_ = y;
    }

    void print(const char* text) override {
        display_buffer_ += text;
        print_calls_.push_back(std::string(text));
    }

    void print(int value) override {
        std::string str = std::to_string(value);
        display_buffer_ += str;
        print_calls_.push_back(str);
    }

    void print(float value, int decimals) override {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        display_buffer_ += buffer;
        print_calls_.push_back(std::string(buffer));
    }

    // Graphics operations
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override {
        line_calls_++;
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        rect_calls_++;
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        fill_rect_calls_++;
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getClearCallCount() const { return clear_call_count_; }
    int getDrawUICallCount() const { return drawUI_call_count_; }
    uint8_t getCurrentBrightness() const { return brightness_; }
    FlightData getLastDrawnData() const { return last_drawn_data_; }
    const std::vector<FlightData>& getDrawHistory() const { return draw_history_; }
    const std::string& getDisplayBuffer() const { return display_buffer_; }
    const std::vector<std::string>& getPrintCalls() const { return print_calls_; }
    uint8_t getCurrentTextSize() const { return current_text_size_; }
    int16_t getCursorX() const { return cursor_x_; }
    int16_t getCursorY() const { return cursor_y_; }
    int getLineCallCount() const { return line_calls_; }
    int getRectCallCount() const { return rect_calls_; }
    int getFillRectCallCount() const { return fill_rect_calls_; }

    void reset() {
        initialize_called_ = false;
        clear_call_count_ = 0;
        drawUI_call_count_ = 0;
        brightness_ = 100;
        last_drawn_data_ = FlightData{};
        draw_history_.clear();
        display_buffer_.clear();
        print_calls_.clear();
        current_text_size_ = 1;
        cursor_x_ = 0;
        cursor_y_ = 0;
        line_calls_ = 0;
        rect_calls_ = 0;
        fill_rect_calls_ = 0;
    }

private:
    bool initialize_called_ = false;
    int clear_call_count_ = 0;
    int drawUI_call_count_ = 0;
    uint8_t brightness_ = 100;
    FlightData last_drawn_data_{};
    std::vector<FlightData> draw_history_;
    
    // New text and graphics state
    std::string display_buffer_;
    std::vector<std::string> print_calls_;
    uint8_t current_text_size_ = 1;
    int16_t cursor_x_ = 0;
    int16_t cursor_y_ = 0;
    int line_calls_ = 0;
    int rect_calls_ = 0;
    int fill_rect_calls_ = 0;
};

#endif // MOCK_DISPLAY_H

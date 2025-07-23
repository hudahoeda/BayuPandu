
#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "../Data/Types.h"
#include <cstdint>

// Display modes for UI screens
enum class DisplayMode {
    Flight,
    Navigation,
    Settings,
    Error
};

class IDisplay {
public:
    virtual ~IDisplay() = default;

    // Basic display operations
    virtual bool initialize() = 0;
    virtual void clear() = 0;
    virtual void setBrightness(uint8_t level) = 0;
    virtual void updateScreen() = 0;
    virtual void showError(const char* message) = 0;
    virtual void setMode(DisplayMode mode) = 0;

    // Legacy method for backward compatibility
    virtual void drawUI(const FlightData& data) = 0;

    // Text operations
    virtual void setTextSize(uint8_t size) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char* text) = 0;
    virtual void print(int value) = 0;
    virtual void print(float value, int decimals = 2) = 0;

    // Graphics operations
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
};

#endif // IDISPLAY_H

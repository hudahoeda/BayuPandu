#pragma once
#include "../HAL/IDisplay.h"
#include <lvgl.h>

#ifdef ARDUINO
#include <Adafruit_ILI9341.h>
#else
class Adafruit_ILI9341;
#endif

class LVGLDisplayDriver : public IDisplay {
public:
    LVGLDisplayDriver(int8_t cs, int8_t dc, int8_t rst);
    virtual ~LVGLDisplayDriver();

    bool initialize() override;
    void clear() override;
    void updateScreen() override;
    void setBrightness(uint8_t level) override;
    void showError(const char* message) override;
    void setMode(DisplayMode mode) override;
    
    // Legacy method for backward compatibility
    void drawUI(const FlightData& data) override;
    
    // Text operations
    void setTextSize(uint8_t size) override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    void print(int value) override;
    void print(float value, int decimals = 2) override;
    
    // Graphics operations
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

private:
    // LVGL display buffer
    lv_disp_draw_buf_t lvglDrawBuffer;
    lv_color_t* lvglBuffer1;
    lv_color_t* lvglBuffer2;
    lv_disp_drv_t lvglDispDrv;

    // Hardware-specific display object
    Adafruit_ILI9341* tft;

    // Pin configuration
    int8_t pinCS;
    int8_t pinDC;
    int8_t pinRST;

    // Display dimensions
    uint16_t width;
    uint16_t height;

    // Brightness level
    uint8_t brightness;

    void lvglFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    void setupLVGL();
};
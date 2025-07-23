
#include "LVGLDisplayDriver.h"
#include "../Data/Types.h"
#include <lvgl.h>

#ifdef ARDUINO
// Only include Arduino-specific libraries when building for Arduino platforms
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#endif

#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  17



LVGLDisplayDriver::LVGLDisplayDriver()
    : lvglBuffer1(nullptr), lvglBuffer2(nullptr), tft(nullptr), pinCS(TFT_CS), pinDC(TFT_DC), pinRST(TFT_RST), width(TFT_WIDTH), height(TFT_HEIGHT), brightness(255)
{
}

LVGLDisplayDriver::~LVGLDisplayDriver() {
    // Destructor implementation
}


bool LVGLDisplayDriver::initialize() {
    setupDisplayHardware();
    setupLVGL();
    return true;
}


void LVGLDisplayDriver::updateScreen() {
    lv_task_handler();
}


void LVGLDisplayDriver::setBrightness(uint8_t level) {
    brightness = level;
    // If your TFT supports PWM backlight, set it here
    // analogWrite(TFT_BACKLIGHT_PIN, brightness);
}


void LVGLDisplayDriver::showError(const char* message) {
    // Display error message using LVGL label
    lv_obj_t* scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, message);
    lv_obj_center(label);
}


void LVGLDisplayDriver::setMode(DisplayMode mode) {
    (void)mode; // Suppress unused parameter warning
    // Switch between display modes (flight, nav, settings)
    // Implement LVGL screen switching logic here
}


void LVGLDisplayDriver::setupLVGL() {
    lv_init();

    // Allocate LVGL display buffers
    lvglBuffer1 = new lv_color_t[width * 40]; // 40 lines buffer
    lvglBuffer2 = new lv_color_t[width * 40];
    lv_disp_draw_buf_init(&lvglDrawBuffer, lvglBuffer1, lvglBuffer2, width * 40);

    // Register display driver
    lv_disp_drv_init(&lvglDispDrv);
    lvglDispDrv.hor_res = width;
    lvglDispDrv.ver_res = height;
    lvglDispDrv.flush_cb = [](lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
        LVGLDisplayDriver* driver = static_cast<LVGLDisplayDriver*>(disp->user_data);
        driver->lvglFlushCallback(disp, area, color_p);
    };
    lvglDispDrv.draw_buf = &lvglDrawBuffer;
    lvglDispDrv.user_data = this;
    lv_disp_drv_register(&lvglDispDrv);
}


void LVGLDisplayDriver::setupDisplayHardware() {
    // Initialize hardware display (SPI, pins, etc.)
#ifdef ARDUINO
    if (!tft) {
        tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
        tft->init(width, height);
        tft->setRotation(0);
        tft->fillScreen(ST77XX_BLACK);
    }
#else
    // Native environment - no hardware initialization needed
#endif
}


void LVGLDisplayDriver::lvglFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    // Transfer LVGL buffer to TFT display
#ifdef ARDUINO
    if (!tft) return;
    for (int y = area->y1; y <= area->y2; ++y) {
        tft->startWrite();
        tft->setAddrWindow(area->x1, y, area->x2 - area->x1 + 1, 1);
        tft->writePixels((uint16_t*)&color_p[(y - area->y1) * (area->x2 - area->x1 + 1)], area->x2 - area->x1 + 1);
        tft->endWrite();
    }
#else
    // Native environment - just simulate flush
    (void)area;
    (void)color_p;
#endif
    lv_disp_flush_ready(disp);
}

void LVGLDisplayDriver::clear() {
    lv_obj_clean(lv_scr_act());
}

void LVGLDisplayDriver::drawUI(const FlightData& data) {
    (void)data; // Suppress unused parameter warning
    // Legacy method - implement basic flight data display using LVGL
    // This can be a simple implementation for backward compatibility
}

void LVGLDisplayDriver::setTextSize(uint8_t size) {
    (void)size; // Suppress unused parameter warning
    // LVGL handles text size through styles, implement if needed
}

void LVGLDisplayDriver::setCursor(int16_t x, int16_t y) {
    (void)x; // Suppress unused parameter warning
    (void)y; // Suppress unused parameter warning
    // LVGL doesn't use cursor concept, but store for legacy compatibility
}

void LVGLDisplayDriver::print(const char* text) {
    (void)text; // Suppress unused parameter warning
    // Legacy print method - could create a temporary label
}

void LVGLDisplayDriver::print(int value) {
    (void)value; // Suppress unused parameter warning
    // Legacy print method - could create a temporary label
}

void LVGLDisplayDriver::print(float value, int decimals) {
    (void)value;    // Suppress unused parameter warning
    (void)decimals; // Suppress unused parameter warning
    // Legacy print method - could create a temporary label  
}

void LVGLDisplayDriver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    (void)color; // Suppress unused parameter warning - LVGL uses styles instead
    // Create LVGL line object
    lv_obj_t* line = lv_line_create(lv_scr_act());
    static lv_point_t points[2];
    points[0].x = x0;
    points[0].y = y0;
    points[1].x = x1;
    points[1].y = y1;
    lv_line_set_points(line, points, 2);
}

void LVGLDisplayDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    (void)color; // Suppress unused parameter warning - LVGL uses styles instead
    // Create LVGL rectangle object
    lv_obj_t* rect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_bg_opa(rect, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(rect, 1, 0);
}

void LVGLDisplayDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    (void)color; // Suppress unused parameter warning - LVGL uses styles instead
    // Create LVGL filled rectangle object
    lv_obj_t* rect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
}

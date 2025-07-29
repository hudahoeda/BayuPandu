#include "LVGLDisplayDriver.h"
#include "../config.h"

#ifdef ARDUINO
#include <Adafruit_ILI9341.h>
#endif

LVGLDisplayDriver::LVGLDisplayDriver(int8_t cs, int8_t dc, int8_t rst)
    : pinCS(cs), 
      pinDC(dc), 
      pinRST(rst), 
      width(240), 
      height(320), 
      brightness(255),
      lvglBuffer1(nullptr), 
      lvglBuffer2(nullptr)
{
#ifdef ARDUINO
    tft = new Adafruit_ILI9341(pinCS, pinDC, pinRST);
#else
    tft = nullptr;
#endif
}

LVGLDisplayDriver::~LVGLDisplayDriver() {
    delete[] lvglBuffer1;
    delete[] lvglBuffer2;
#ifdef ARDUINO
    delete tft;
#endif
}

bool LVGLDisplayDriver::initialize() {
#ifdef ARDUINO
    tft->begin();
    tft->setRotation(0);
#endif
    setupLVGL();
    return true;
}

void LVGLDisplayDriver::clear() {
    lv_obj_clean(lv_scr_act());
}

void LVGLDisplayDriver::updateScreen() {
    lv_task_handler();
}

void LVGLDisplayDriver::setBrightness(uint8_t level) {
    brightness = level;
    // You may need to control the backlight pin here, e.g., analogWrite(TFT_BL, level);
}

void LVGLDisplayDriver::showError(const char* message) {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, message);
    lv_obj_center(label);
}

void LVGLDisplayDriver::setMode(DisplayMode mode) {
    // Implement screen switching logic here based on the mode
}

void LVGLDisplayDriver::drawUI(const FlightData& data) {
    // Legacy method, can be implemented with LVGL widgets
}

void LVGLDisplayDriver::setTextSize(uint8_t size) {
    // LVGL handles text size via styles
}

void LVGLDisplayDriver::setCursor(int16_t x, int16_t y) {
    // LVGL does not use a cursor in the same way
}

void LVGLDisplayDriver::print(const char* text) {
    // Legacy method
}

void LVGLDisplayDriver::print(int value) {
    // Legacy method
}

void LVGLDisplayDriver::print(float value, int decimals) {
    // Legacy method
}

void LVGLDisplayDriver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    // LVGL implementation
}

void LVGLDisplayDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // LVGL implementation
}

void LVGLDisplayDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // LVGL implementation
}

void LVGLDisplayDriver::setupLVGL() {
    lvglBuffer1 = new lv_color_t[width * 40];
    lvglBuffer2 = new lv_color_t[width * 40];
    lv_disp_draw_buf_init(&lvglDrawBuffer, lvglBuffer1, lvglBuffer2, width * 40);

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

void LVGLDisplayDriver::lvglFlushCallback(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
#ifdef ARDUINO
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    if (tft) {
        tft->startWrite();
        tft->setAddrWindow(area->x1, area->y1, w, h);
        tft->writePixels((uint16_t*)color_p, w * h);
        tft->endWrite();
    }
#endif
    lv_disp_flush_ready(disp);
}
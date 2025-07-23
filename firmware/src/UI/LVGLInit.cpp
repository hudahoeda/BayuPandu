#include "LVGLInit.h"
#ifdef ARDUINO
#include "Arduino.h"
#else
#include <ArduinoFake.h>
#endif

bool LVGLInit::initialized = false;
static uint32_t lastTick = 0;

bool LVGLInit::initialize() {
    if (initialized) {
        return true;
    }
    
    // Initialize LVGL
    lv_init();
    
    // Initialize tick tracking
    lastTick = millis();
    
    // Note: Display driver and input driver will be implemented in tasks 7.2 and 7.3
    // For now, we just initialize the core LVGL system
    
    initialized = true;
    return true;
}

void LVGLInit::tick() {
    if (!initialized) {
        return;
    }
    
    // Manual tick handling since LV_TICK_CUSTOM is disabled
    uint32_t currentTick = millis();
    uint32_t elapsed = currentTick - lastTick;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        lastTick = currentTick;
    }
}

void LVGLInit::handler() {
    if (!initialized) {
        return;
    }
    
    // Update tick first
    tick();
    
    // Handle LVGL tasks
    lv_timer_handler();
}

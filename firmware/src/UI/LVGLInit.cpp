#include "LVGLInit.h"
#include "UI/UserInterface.h"
#ifdef ARDUINO
#include "Arduino.h"
#else
#include <ArduinoFake.h>
#endif

bool LVGLInit::initialized = false;
InputManager* LVGLInit::inputManager = nullptr;
LVGLInputDriver* LVGLInit::lvglInputDriver = nullptr;
static uint32_t lastTick = 0;

bool LVGLInit::initialize(IArduino& arduino) {
    if (initialized) {
        return true;
    }
    
    // Initialize LVGL
    lv_init();
    
    // Initialize tick tracking
    lastTick = millis();
    
    // Initialize InputManager and LVGLInputDriver
    inputManager = new InputManager(arduino);
    inputManager->initPins(); // Initialize button pins
    
    lvglInputDriver = new LVGLInputDriver(*inputManager);
    lvglInputDriver->initialize(); // Register LVGL input driver

    initialized = true;
    return true;
}

void LVGLInit::setUserInterface(UserInterface* ui) {
    if (lvglInputDriver) {
        lvglInputDriver->setUserInterface(ui);
    }
}

void LVGLInit::handler() {
    if (!initialized) {
        return;
    }
    
    // Update InputManager state
    inputManager->update();
    
    // Handle LVGL tasks (LVGL's custom tick source handles timing)
    lv_timer_handler();
}

#pragma once

#include "lvgl.h"
#include "InputManager.h"

// Forward declaration to avoid circular dependency
class UserInterface;

class LVGLInputDriver {
public:
    LVGLInputDriver(InputManager& inputManager);
    bool initialize();
    
    // Set the UserInterface to handle button actions
    void setUserInterface(UserInterface* ui);

private:
    InputManager& _inputManager;
    UserInterface* _userInterface;
    lv_indev_drv_t _indev_drv;
    lv_indev_t* _indev;
    
    // Button state tracking for action detection
    bool lastButtonState[InputManager::MAX_BUTTONS];
    uint32_t buttonPressTime[InputManager::MAX_BUTTONS];
    bool longPressTriggered[InputManager::MAX_BUTTONS];

    static void lvgl_indev_read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data);
    
    // Button action detection
    void updateButtonStates();
    
    // Map button IDs to LVGL key codes
    lv_key_t getLVGLKeyCode(uint8_t buttonId);
};

#include "LVGLInputDriver.h"
#include "UI/UserInterface.h"
#include "lvgl.h"

LVGLInputDriver::LVGLInputDriver(InputManager& inputManager)
    : _inputManager(inputManager), _userInterface(nullptr) {
    // Initialize the LVGL input device driver
    lv_indev_drv_init(&_indev_drv);
    _indev_drv.type = LV_INDEV_TYPE_BUTTON;
    _indev_drv.read_cb = lvgl_indev_read_cb;
    _indev_drv.user_data = this; // Pass 'this' pointer to the static callback
    
    // Initialize button state tracking
    for (int i = 0; i < InputManager::MAX_BUTTONS; i++) {
        lastButtonState[i] = false;
        buttonPressTime[i] = 0;
        longPressTriggered[i] = false;
    }
}

void LVGLInputDriver::setUserInterface(UserInterface* ui) {
    _userInterface = ui;
}

bool LVGLInputDriver::initialize() {
    _indev = lv_indev_drv_register(&_indev_drv);

    // Set a group for the buttons, if needed for focus management
    // lv_group_t* g = lv_group_create();
    // lv_indev_set_group(_indev, g);

    // Assign points to the buttons for LVGL's button input.
    // These coordinates should correspond to points on the screen that LVGL will "click"
    // when a button is pressed. For now, we'll use arbitrary points.
    static lv_point_t points[InputManager::MAX_BUTTONS] = {
        {10, 10},  // Button 0 (e.g., UP)
        {10, 50},  // Button 1 (e.g., DOWN)
        {50, 10},  // Button 2 (e.g., LEFT)
        {50, 50}   // Button 3 (e.g., RIGHT/SELECT)
    };
    lv_indev_set_button_points(_indev, points);

    return _indev != nullptr;
}

void LVGLInputDriver::lvgl_indev_read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
    LVGLInputDriver* driver = static_cast<LVGLInputDriver*>(indev_drv->user_data);

    data->state = LV_INDEV_STATE_RELEASED; // Default to released

    // Update button states and handle actions
    driver->updateButtonStates();

    // Iterate through all buttons for LVGL input
    for (uint8_t i = 0; i < InputManager::MAX_BUTTONS; i++) {
        if (driver->_inputManager.getButtonState(i)) {
            data->state = LV_INDEV_STATE_PRESSED;
            data->btn_id = i; // Set the button ID that is currently pressed
            break; // Only one button can be pressed at a time for this simple example
        }
    }
}

void LVGLInputDriver::updateButtonStates() {
    if (!_userInterface) return;
    
    uint32_t currentTime = lv_tick_get();
    const uint32_t LONG_PRESS_TIME = 1000; // 1 second for long press
    
    for (uint8_t i = 0; i < InputManager::MAX_BUTTONS; i++) {
        bool currentState = _inputManager.getButtonState(i);
        bool lastState = lastButtonState[i];
        
        if (currentState && !lastState) {
            // Button just pressed
            buttonPressTime[i] = currentTime;
            longPressTriggered[i] = false;
        } else if (!currentState && lastState) {
            // Button just released
            if (!longPressTriggered[i]) {
                // Short press
                _userInterface->handleButtonAction(ButtonAction::SHORT_PRESS, i);
            }
            longPressTriggered[i] = false;
        } else if (currentState && lastState) {
            // Button held down
            if (!longPressTriggered[i] && (currentTime - buttonPressTime[i]) >= LONG_PRESS_TIME) {
                // Long press triggered
                _userInterface->handleButtonAction(ButtonAction::LONG_PRESS, i);
                longPressTriggered[i] = true;
            }
        }
        
        lastButtonState[i] = currentState;
    }
}

lv_key_t LVGLInputDriver::getLVGLKeyCode(uint8_t buttonId) {
    // This function can be expanded to map buttons to specific LVGL key codes
    // For now, it's a simple mapping.
    switch (buttonId) {
        case 0: return LV_KEY_UP;
        case 1: return LV_KEY_DOWN;
        case 2: return LV_KEY_LEFT;
        case 3: return LV_KEY_ENTER; // or LV_KEY_RIGHT if you prefer
        default: return LV_KEY_ESC; // Use LV_KEY_ESC instead of LV_KEY_NONE
    }
}

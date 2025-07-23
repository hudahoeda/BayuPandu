#pragma once

#include "lvgl.h"
#include "InputManager.h"

class LVGLInputDriver {
public:
    LVGLInputDriver(InputManager& inputManager);
    bool initialize();

private:
    InputManager& _inputManager;
    lv_indev_drv_t _indev_drv;
    lv_indev_t* _indev;

    static void lvgl_indev_read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data);
    
    // Map button IDs to LVGL key codes
    lv_key_t getLVGLKeyCode(uint8_t buttonId);
};

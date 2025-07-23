#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include "lvgl.h"
#include "InputManager.h"
#include "LVGLInputDriver.h"
#include "HAL/IArduino.h"

class LVGLInit {
public:
    static bool initialize(IArduino& arduino);
    static void handler();
    
private:
    static bool initialized;
    static InputManager* inputManager;
    static LVGLInputDriver* lvglInputDriver;
};

#endif // LVGL_INIT_H

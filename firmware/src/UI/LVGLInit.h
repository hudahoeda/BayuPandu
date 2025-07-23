#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include "lvgl.h"
#include "InputManager.h"
#include "LVGLInputDriver.h"
#include "HAL/IArduino.h"

// Forward declaration
class UserInterface;

class LVGLInit {
public:
    static bool initialize(IArduino& arduino);
    static void handler();
    static void setUserInterface(UserInterface* ui);
    
private:
    static bool initialized;
    static InputManager* inputManager;
    static LVGLInputDriver* lvglInputDriver;
};

#endif // LVGL_INIT_H

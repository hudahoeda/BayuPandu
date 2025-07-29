#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include "lvgl.h"
#include "InputManager.h"
#include "LVGLInputDriver.h"
#include "LVGLDisplayDriver.h"
#include "HAL/IArduino.h"

// Forward declaration
class UserInterface;

class LVGLInit {
public:
    static bool initialize(IArduino& arduino);
    static void handler();
    static void setUserInterface(UserInterface* ui);
    
public:
    static InputManager* inputManager;
private:
    static bool initialized;
    static LVGLInputDriver* lvglInputDriver;
    static LVGLDisplayDriver* displayDriver;
};

#endif // LVGL_INIT_H

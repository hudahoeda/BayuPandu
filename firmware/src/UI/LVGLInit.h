#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include "lvgl.h"

class LVGLInit {
public:
    static bool initialize();
    static void tick();
    static void handler();
    
private:
    static bool initialized;
};

#endif // LVGL_INIT_H

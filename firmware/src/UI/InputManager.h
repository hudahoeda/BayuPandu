#pragma once

#include "HAL/IArduino.h"
#include "Data/Types.h"
#include <functional>

class InputManager {
public:
    static const uint8_t MAX_BUTTONS = 4;
    
    explicit InputManager(IArduino& arduino);
    
    void initPins();
    void update();
    bool getButtonState(uint8_t buttonId) const;
    char getSerialAction(); // For Wokwi simulation control

private:
    IArduino& arduino;
    
    // Button state tracking
    bool buttonStates[MAX_BUTTONS];
    uint32_t lastButtonChangeTime[MAX_BUTTONS];
    
    // Timing configuration
    uint32_t debounceDelay;
    
    uint8_t getButtonPin(uint8_t buttonId);
};

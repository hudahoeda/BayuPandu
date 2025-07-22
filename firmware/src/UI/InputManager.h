#pragma once

#include "HAL/IArduino.h"
#include "Data/Types.h"
#include <functional>

using ButtonActionCallback = std::function<void(ButtonAction, uint8_t)>;

class InputManager {
public:
    static const uint8_t MAX_BUTTONS = 4;
    
    explicit InputManager(IArduino& arduino);
    
    void update();
    void setButtonActionCallback(ButtonActionCallback callback);

private:
    IArduino& arduino;
    ButtonActionCallback buttonActionCallback = nullptr;
    
    // Button state tracking
    bool buttonStates[MAX_BUTTONS];
    uint32_t lastButtonPress[MAX_BUTTONS];
    bool buttonPressed[MAX_BUTTONS];
    uint32_t buttonPressTime[MAX_BUTTONS];
    
    // Timing configuration
    uint32_t debounceDelay;
    uint32_t longPressDelay;
    
    void onButtonAction(ButtonAction action, uint8_t buttonId);
    uint8_t getButtonPin(uint8_t buttonId);
};

#include "InputManager.h"

#include "InputManager.h"
#include <cstring>

InputManager::InputManager(IArduino& arduino) 
    : arduino(arduino), debounceDelay(50), longPressDelay(1000) {
    // Initialize button states
    memset(buttonStates, 0, sizeof(buttonStates));
    memset(lastButtonPress, 0, sizeof(lastButtonPress));
    memset(buttonPressed, false, sizeof(buttonPressed));
}

void InputManager::update() {
    uint32_t currentTime = arduino.millis();
    
    for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
        bool currentState = arduino.digitalRead(getButtonPin(i));
        
        // Handle button state changes
        if (currentState != buttonStates[i]) {
            if (currentTime - lastButtonPress[i] > debounceDelay) {
                buttonStates[i] = currentState;
                lastButtonPress[i] = currentTime;
                
                if (currentState) {
                    // Button pressed
                    buttonPressed[i] = true;
                    buttonPressTime[i] = currentTime;
                }
            }
        }
        
        // Check for button releases and long presses
        if (buttonPressed[i]) {
            if (!buttonStates[i]) {
                // Button released
                uint32_t pressDuration = currentTime - buttonPressTime[i];
                if (pressDuration >= longPressDelay) {
                    onButtonAction(ButtonAction::LONG_PRESS, i);
                } else {
                    onButtonAction(ButtonAction::SHORT_PRESS, i);
                }
                buttonPressed[i] = false;
            } else if (currentTime - buttonPressTime[i] >= longPressDelay) {
                // Long press detected (still holding)
                onButtonAction(ButtonAction::LONG_PRESS, i);
                buttonPressed[i] = false; // Prevent repeated long press events
            }
        }
    }
}

void InputManager::setButtonActionCallback(ButtonActionCallback callback) {
    buttonActionCallback = callback;
}

void InputManager::onButtonAction(ButtonAction action, uint8_t buttonId) {
    if (buttonActionCallback) {
        buttonActionCallback(action, buttonId);
    }
}

uint8_t InputManager::getButtonPin(uint8_t buttonId) {
    // Map button IDs to GPIO pins - adjust these according to your hardware
    switch (buttonId) {
        case 0: return 32; // UP button
        case 1: return 33; // DOWN button  
        case 2: return 25; // LEFT button
        case 3: return 26; // RIGHT/SELECT button
        default: return 0;
    }
}

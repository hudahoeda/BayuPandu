#include "InputManager.h"

#include "InputManager.h"
#include <cstring> // For memset

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <ArduinoFake.h>
#endif

InputManager::InputManager(IArduino& arduino) 
    : arduino(arduino), debounceDelay(50) {
    // Initialize button states
    memset(buttonStates, 0, sizeof(buttonStates));
    memset(lastButtonChangeTime, 0, sizeof(lastButtonChangeTime));
}

void InputManager::initPins() {
    for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
        arduino.pinMode(getButtonPin(i), INPUT_PULLUP); // Assuming pull-up resistors
    }
}

void InputManager::update() {
    uint32_t currentTime = arduino.millis();
    
    for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
        // Read the current state of the button (LOW if pressed, HIGH if not pressed due to pull-up)
        bool rawState = (arduino.digitalRead(getButtonPin(i)) == LOW); 
        
        // Debounce logic
        if (rawState != buttonStates[i]) {
            if (currentTime - lastButtonChangeTime[i] > debounceDelay) {
                buttonStates[i] = rawState;
                lastButtonChangeTime[i] = currentTime;
            }
        }
    }
}

bool InputManager::getButtonState(uint8_t buttonId) const {
    if (buttonId >= MAX_BUTTONS) {
        return false; // Invalid button ID
    }
    return buttonStates[buttonId];
}

uint8_t InputManager::getButtonPin(uint8_t buttonId) {
    // Map button IDs to GPIO pins - adjust these according to your hardware
    switch (buttonId) {
        case 0: return 32; // UP button
        case 1: return 33; // DOWN button  
        case 2: return 25; // LEFT button
        case 3: return 26; // RIGHT/SELECT button
        default: return 0; // Should not happen with MAX_BUTTONS check
    }
}

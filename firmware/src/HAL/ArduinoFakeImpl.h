#ifndef ARDUINO_FAKE_IMPL_H
#define ARDUINO_FAKE_IMPL_H

#include "HAL/IArduino.h"

#ifndef ARDUINO
#include <ArduinoFake.h>

/**
 * ArduinoFake implementation of IArduino interface.
 * Uses ArduinoFake for testing without real hardware.
 */
class ArduinoFakeImpl : public IArduino {
public:
    uint32_t millis() override {
        return ::millis();
    }

    void delay(uint32_t ms) override {
        ::delay(ms);
    }

    void pinMode(uint8_t pin, uint8_t mode) override {
        ::pinMode(pin, mode);
    }

    void digitalWrite(uint8_t pin, uint8_t value) override {
        ::digitalWrite(pin, value);
    }

    int digitalRead(uint8_t pin) override {
        return ::digitalRead(pin);
    }

    void serialBegin(uint32_t baud) override {
        Serial.begin(baud);
    }

    void serialPrint(const char* text) override {
        Serial.print(text);
    }

    void serialPrintln(const char* text) override {
        Serial.println(text);
    }

    int serialAvailable() override {
        return 0; // No serial data available in fake implementation
    }

    int serialRead() override {
        return -1; // No serial data available in fake implementation
    }

    uint8_t getLedBuiltinPin() override {
        return LED_BUILTIN;
    }
};

#endif // !ARDUINO

#endif // ARDUINO_FAKE_IMPL_H

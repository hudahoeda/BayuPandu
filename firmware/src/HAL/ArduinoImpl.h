#ifndef ARDUINO_IMPL_H
#define ARDUINO_IMPL_H

#include "HAL/IArduino.h"

#ifdef ARDUINO
#include <Arduino.h>

/**
 * Real Arduino implementation of IArduino interface.
 * Uses actual Arduino framework functions.
 */
class ArduinoImpl : public IArduino {
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
        return Serial.available();
    }

    int serialRead() override {
        return Serial.read();
    }

    uint8_t getLedBuiltinPin() override {
        return LED_BUILTIN;
    }
};

#endif // ARDUINO

#endif // ARDUINO_IMPL_H

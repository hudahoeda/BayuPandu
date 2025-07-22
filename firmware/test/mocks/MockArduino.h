#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include "HAL/IArduino.h"
#include <string>
#include <array>
#include <iostream>

/**
 * Mock Arduino implementation for unit testing.
 * Provides controllable behavior and state inspection.
 */
class MockArduino : public IArduino {
public:
    MockArduino() { reset(); }

    uint32_t millis() override {
        return current_millis_;
    }

    void delay(uint32_t ms) override {
        current_millis_ += ms;
        delay_call_count_++;
    }

    void pinMode(uint8_t pin, uint8_t mode) override {
        if (pin < pin_modes_.size()) {
            pin_modes_[pin] = mode;
            pinMode_call_count_++;
        }
    }

    void digitalWrite(uint8_t pin, uint8_t value) override {
        if (pin < pin_values_.size()) {
            pin_values_[pin] = value;
            digitalWrite_call_count_++;
        }
    }

    int digitalRead(uint8_t pin) override {
        digitalRead_call_count_++;
        if (pin < pin_values_.size()) {
            return pin_values_[pin];
        }
        return IArduino::LOW_VALUE;
    }

    void serialBegin(uint32_t baud) override {
        serial_baud_ = baud;
        serial_begin_called_ = true;
    }

    void serialPrint(const char* text) override {
        serial_output_ += text;
        serialPrint_call_count_++;
    }

    void serialPrintln(const char* text) override {
        serial_output_ += text;
        serial_output_ += "\n";
        serialPrintln_call_count_++;
    }

    uint8_t getLedBuiltinPin() override {
        return 13; // Mock LED pin (ArduinoFake default)
    }

    // Test helper methods for inspection
    void setMillis(uint32_t time) { current_millis_ = time; }
    uint32_t getCurrentMillis() const { return current_millis_; }
    
    uint8_t getPinMode(uint8_t pin) const { 
        return pin < pin_modes_.size() ? pin_modes_[pin] : 0; 
    }
    
    uint8_t getPinValue(uint8_t pin) const { 
        return pin < pin_values_.size() ? pin_values_[pin] : 0; 
    }
    
    void setPinValue(uint8_t pin, uint8_t value) {
        if (pin < pin_values_.size()) {
            pin_values_[pin] = value;
        }
    }
    
    bool wasSerialBeginCalled() const { return serial_begin_called_; }
    uint32_t getSerialBaud() const { return serial_baud_; }
    const std::string& getSerialOutput() const { return serial_output_; }
    
    // Call count getters for verification
    int getDelayCallCount() const { return delay_call_count_; }
    int getPinModeCallCount() const { return pinMode_call_count_; }
    int getDigitalWriteCallCount() const { return digitalWrite_call_count_; }
    int getDigitalReadCallCount() const { return digitalRead_call_count_; }
    int getSerialPrintCallCount() const { return serialPrint_call_count_; }
    int getSerialPrintlnCallCount() const { return serialPrintln_call_count_; }

    void reset() {
        current_millis_ = 0;
        pin_modes_.fill(0);
        pin_values_.fill(IArduino::LOW_VALUE);
        serial_begin_called_ = false;
        serial_baud_ = 0;
        serial_output_.clear();
        
        // Reset call counters
        delay_call_count_ = 0;
        pinMode_call_count_ = 0;
        digitalWrite_call_count_ = 0;
        digitalRead_call_count_ = 0;
        serialPrint_call_count_ = 0;
        serialPrintln_call_count_ = 0;
    }

private:
    uint32_t current_millis_ = 0;
    std::array<uint8_t, 256> pin_modes_{};
    std::array<uint8_t, 256> pin_values_{};
    bool serial_begin_called_ = false;
    uint32_t serial_baud_ = 0;
    std::string serial_output_;
    
    // Call counters for verification
    int delay_call_count_ = 0;
    int pinMode_call_count_ = 0;
    int digitalWrite_call_count_ = 0;
    int digitalRead_call_count_ = 0;
    int serialPrint_call_count_ = 0;
    int serialPrintln_call_count_ = 0;
};

#endif // MOCK_ARDUINO_H

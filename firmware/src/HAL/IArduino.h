#ifndef IARDUINO_H
#define IARDUINO_H

#include <cstdint>

/**
 * Interface for abstracting Arduino framework dependencies.
 * This allows for easier testing and better separation of concerns.
 */
class IArduino {
public:
    virtual ~IArduino() = default;

    // Time functions
    virtual uint32_t millis() = 0;
    virtual void delay(uint32_t ms) = 0;

    // Digital I/O
    virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
    virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;
    virtual int digitalRead(uint8_t pin) = 0;

    // Serial communication
    virtual void serialBegin(uint32_t baud) = 0;
    virtual void serialPrint(const char* text) = 0;
    virtual void serialPrintln(const char* text) = 0;

    // Pin definitions
    virtual uint8_t getLedBuiltinPin() = 0;

    // Pin modes (constants that match Arduino)
    static constexpr uint8_t OUTPUT_MODE = 0x01;
    static constexpr uint8_t INPUT_MODE = 0x00;
    static constexpr uint8_t INPUT_PULLUP_MODE = 0x02;
    
    // Digital values (constants that match Arduino)
    static constexpr uint8_t HIGH_VALUE = 0x1;
    static constexpr uint8_t LOW_VALUE = 0x0;
};

#endif // IARDUINO_H

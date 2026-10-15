#pragma once
// Minimal Arduino stand-in for the desktop simulator. Only what the firmware's
// non-ARDUINO code paths reach for directly; everything else goes through IArduino.

#include <cstdint>
#include <cstddef>

#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x00
#define OUTPUT 0x01
#define INPUT_PULLUP 0x02

uint32_t millis();
void delay(uint32_t ms);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
int digitalRead(uint8_t pin);

struct SimSerial {
    void begin(unsigned long) {}
    void print(const char* s);
    void println(const char* s);
    int available() { return 0; }
    int read() { return -1; }
};
extern SimSerial Serial;

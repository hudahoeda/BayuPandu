#ifdef ARDUINO
#include <Arduino.h>
#include "HAL/ArduinoImpl.h"
#else
#include <ArduinoFake.h>
#include "HAL/ArduinoFakeImpl.h"
#endif

#include "config.h"
#include "HAL/IArduino.h"

// Global Arduino abstraction
IArduino* arduino_impl = nullptr;

void setup() {
#ifdef ARDUINO
  arduino_impl = new ArduinoImpl();
#else
  arduino_impl = new ArduinoFakeImpl();
#endif

  // Initialize serial communication
  arduino_impl->serialBegin(115200);
  
  // Set the LED pin as an output
  arduino_impl->pinMode(arduino_impl->getLedBuiltinPin(), IArduino::OUTPUT_MODE);
}

void loop() {
  // Turn the LED on
  arduino_impl->digitalWrite(arduino_impl->getLedBuiltinPin(), IArduino::HIGH_VALUE);
  arduino_impl->serialPrintln("LED is ON");
  arduino_impl->delay(1000);

  // Turn the LED off
  arduino_impl->digitalWrite(arduino_impl->getLedBuiltinPin(), IArduino::LOW_VALUE);
  arduino_impl->serialPrintln("LED is OFF");
  arduino_impl->delay(1000);
}

#ifndef ARDUINO
// For native testing, provide a main function
int main() {
  setup();
  for (int i = 0; i < 5; i++) {  // Run loop a few times for testing
    loop();
  }
  delete arduino_impl;
  return 0;
}
#endif

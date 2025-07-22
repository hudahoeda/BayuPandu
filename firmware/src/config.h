#ifndef CONFIG_H
#define CONFIG_H

// Pin definition for the onboard LED on most ESP32 dev boards
// Only define if not already defined (avoid conflict with ArduinoFake)
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#endif // CONFIG_H

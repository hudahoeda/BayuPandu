#include <Arduino.h>
#include "config.h"

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Set the LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Turn the LED on
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("LED is ON");
  delay(1000);

  // Turn the LED off
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("LED is OFF");
  delay(1000);
}

#pragma once
// The firmware includes <ArduinoFake.h> on non-ARDUINO builds; the simulator
// serves the same globals from its own shim instead of the mocking library.
#include "Arduino.h"

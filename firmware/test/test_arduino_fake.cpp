#include <ArduinoFake.h>
#include "HAL/ArduinoFakeImpl.h"

using namespace fakeit;

void test_arduino_fake_impl() {
    // Reset ArduinoFake state
    ArduinoFakeReset();

    // Create the implementation
    ArduinoFakeImpl arduino;

    // Test millis
    When(Method(ArduinoFake(), millis)).Return(1000, 2000, 3000);
    
    auto time1 = arduino.millis();
    auto time2 = arduino.millis();
    auto time3 = arduino.millis();
    
    // Verify the sequence
    Verify(Method(ArduinoFake(), millis)).Exactly(3_Times);
    
    // Test digitalWrite and pinMode
    When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
    When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
    
    arduino.pinMode(13, IArduino::OUTPUT_MODE);
    arduino.digitalWrite(13, IArduino::HIGH_VALUE);
    arduino.digitalWrite(13, IArduino::LOW_VALUE);
    
    Verify(Method(ArduinoFake(), pinMode).Using(13, IArduino::OUTPUT_MODE)).Once();
    Verify(Method(ArduinoFake(), digitalWrite).Using(13, IArduino::HIGH_VALUE)).Once();
    Verify(Method(ArduinoFake(), digitalWrite).Using(13, IArduino::LOW_VALUE)).Once();
    
    // Test delay
    When(Method(ArduinoFake(), delay)).AlwaysReturn();
    arduino.delay(1000);
    Verify(Method(ArduinoFake(), delay).Using(1000)).Once();
    
    // Test Serial functions
    When(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char*))).AlwaysReturn();
    
    arduino.serialBegin(115200);
    arduino.serialPrint("Test");
    arduino.serialPrintln("Line");
    
    Verify(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long)).Using(115200)).Once();
    Verify(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*)).Using("Test")).Once();
    Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char*)).Using("Line")).Once();
}

int main() {
    test_arduino_fake_impl();
    return 0;
}

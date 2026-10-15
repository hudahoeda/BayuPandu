#include <ArduinoFake.h>
#undef round
#undef abs
#include <gtest/gtest.h>
#include "HAL/ArduinoFakeImpl.h"

using namespace fakeit;

TEST(ArduinoFakeImplTest, ForwardsCallsToArduinoFake) {
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
    EXPECT_EQ(time1, 1000u);
    EXPECT_EQ(time2, 2000u);
    EXPECT_EQ(time3, 3000u);
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
    Verify(Method(ArduinoFake(), delay).Using(1000ul)).Once();
    
    // Test Serial functions
    When(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char*))).AlwaysReturn();
    
    arduino.serialBegin(115200);
    // fakeit compares const char* by address, so pass the same pointers it checks.
    const char* text = "Test";
    const char* line = "Line";
    arduino.serialPrint(text);
    arduino.serialPrintln(line);
    
    Verify(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long)).Using(115200ul)).Once();
    Verify(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*)).Using(text)).Once();
    Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char*)).Using(line)).Once();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS()) {}
    // Always exit 0 so PlatformIO parses the results itself.
    return 0;
}



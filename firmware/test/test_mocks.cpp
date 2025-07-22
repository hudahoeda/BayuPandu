#include <gtest/gtest.h>
#include <ArduinoFake.h>
#include "mocks/MockAudio.h"
#include "mocks/MockBarometer.h"
#include "mocks/MockDisplay.h"
#include "mocks/MockGPS.h"
#include "mocks/MockIMU.h"
#include "mocks/MockPower.h"
#include "mocks/MockStorage.h"

// --- MockAudio Tests ---
class MockAudioTest : public ::testing::Test {
protected:
    MockAudio audio;
    void SetUp() override { audio.reset(); }
};

TEST_F(MockAudioTest, Initialization) {
    EXPECT_FALSE(audio.wasInitializeCalled());
    audio.initialize();
    EXPECT_TRUE(audio.wasInitializeCalled());
}

TEST_F(MockAudioTest, ToneGeneration) {
    EXPECT_EQ(audio.getToneCallCount(), 0);
    audio.tone(440, 1000);
    EXPECT_EQ(audio.getToneCallCount(), 1);
    EXPECT_TRUE(audio.isToneActive());
    EXPECT_EQ(audio.getLastTone().frequency, 440);
    audio.noTone();
    EXPECT_FALSE(audio.isToneActive());
    EXPECT_EQ(audio.getNoToneCallCount(), 1);
}

// --- MockBarometer Tests ---
class MockBarometerTest : public ::testing::Test {
protected:
    MockBarometer barometer;
    void SetUp() override { barometer.reset(); }
};

TEST_F(MockBarometerTest, Initialization) {
    EXPECT_FALSE(barometer.wasInitializeCalled());
    barometer.initialize();
    EXPECT_TRUE(barometer.wasInitializeCalled());
}

TEST_F(MockBarometerTest, ReadData) {
    barometer.setNextPressure(1010.5f);
    barometer.setNextTemperature(22.5f);
    float pressure, temp;
    barometer.readPressure(pressure);
    barometer.readTemperature(temp);
    EXPECT_EQ(pressure, 1010.5f);
    EXPECT_EQ(temp, 22.5f);
    EXPECT_EQ(barometer.getReadPressureCallCount(), 1);
    EXPECT_EQ(barometer.getReadTemperatureCallCount(), 1);
}

// --- MockDisplay Tests ---
class MockDisplayTest : public ::testing::Test {
protected:
    MockDisplay display;
    void SetUp() override { display.reset(); }
};

TEST_F(MockDisplayTest, Initialization) {
    EXPECT_FALSE(display.wasInitializeCalled());
    display.initialize();
    EXPECT_TRUE(display.wasInitializeCalled());
}

TEST_F(MockDisplayTest, Drawing) {
    EXPECT_EQ(display.getDrawUICallCount(), 0);
    FlightData data{};
    display.drawUI(data);
    EXPECT_EQ(display.getDrawUICallCount(), 1);
    display.clear();
    EXPECT_EQ(display.getClearCallCount(), 1);
}

// --- MockGPS Tests ---
class MockGPSTest : public ::testing::Test {
protected:
    MockGPS gps;
    void SetUp() override { gps.reset(); }
};

TEST_F(MockGPSTest, Initialization) {
    EXPECT_FALSE(gps.wasInitializeCalled());
    gps.initialize();
    EXPECT_TRUE(gps.wasInitializeCalled());
}

TEST_F(MockGPSTest, FixStatus) {
    EXPECT_FALSE(gps.hasValidFix());
    gps.setFixStatus(true);
    EXPECT_TRUE(gps.hasValidFix());
}

// --- MockIMU Tests ---
class MockIMUTest : public ::testing::Test {
protected:
    MockIMU imu;
    void SetUp() override { imu.reset(); }
};

TEST_F(MockIMUTest, Initialization) {
    EXPECT_FALSE(imu.wasInitializeCalled());
    imu.initialize();
    EXPECT_TRUE(imu.wasInitializeCalled());
}

TEST_F(MockIMUTest, Calibration) {
    EXPECT_FALSE(imu.isCalibrated());
    imu.calibrate();
    EXPECT_TRUE(imu.isCalibrated());
}

// --- MockPower Tests ---
class MockPowerTest : public ::testing::Test {
protected:
    MockPower power;
    void SetUp() override { power.reset(); }
};

TEST_F(MockPowerTest, Initialization) {
    EXPECT_FALSE(power.wasInitializeCalled());
    power.initialize();
    EXPECT_TRUE(power.wasInitializeCalled());
}

TEST_F(MockPowerTest, BatteryVoltage) {
    power.setVoltage(3.8f);
    EXPECT_EQ(power.getBatteryVoltage(), 3.8f);
    EXPECT_GT(power.getBatteryPercentage(), 0);
}

// --- MockStorage Tests ---
class MockStorageTest : public ::testing::Test {
protected:
    MockStorage storage;
    void SetUp() override { storage.reset(); }
};

TEST_F(MockStorageTest, Initialization) {
    EXPECT_FALSE(storage.wasInitializeCalled());
    storage.initialize();
    EXPECT_TRUE(storage.wasInitializeCalled());
}

TEST_F(MockStorageTest, FileOperations) {
    const char* path = "/test.txt";
    const char* content = "hello";
    EXPECT_FALSE(storage.fileExists(path));
    storage.writeFile(path, content);
    EXPECT_TRUE(storage.fileExists(path));
    char buffer[10];
    storage.readFile(path, buffer, 10);
    EXPECT_STREQ(buffer, content);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

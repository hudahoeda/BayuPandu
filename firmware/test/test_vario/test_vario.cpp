#include <gtest/gtest.h>
#include <cmath>
#include <random>
#include "Services/VarioFilter.h"
#include "Services/VariometerService.h"
#include "mocks/MockArduino.h"
#include "mocks/MockAudio.h"
#include "mocks/MockBarometer.h"

namespace {

constexpr float kDt = 0.05f;  // 20 Hz, a realistic MS5611 rate at full oversampling

// Altitude samples of a pilot moving at a steady vertical speed, with Gaussian
// sensor noise. Seeded, so every run sees the same numbers.
struct Flight {
    std::mt19937 rng{42};
    std::normal_distribution<float> noise;
    float altitude;

    Flight(float startAltitude, float noiseSigma) : noise(0.0f, noiseSigma), altitude(startAltitude) {}

    float sample(float verticalSpeed) {
        altitude += verticalSpeed * kDt;
        return altitude + noise(rng);
    }
};

// Runs `seconds` of flight and returns the RMS error of the climb estimate
// over the last half, after the filter has had time to settle.
float runAndMeasure(VarioFilter& f, Flight& flight, float verticalSpeed, float seconds) {
    int steps = static_cast<int>(seconds / kDt);
    double sq = 0;
    int n = 0;
    for (int i = 0; i < steps; i++) {
        f.update(flight.sample(verticalSpeed), kDt);
        if (i >= steps / 2) {
            double e = f.verticalSpeed() - verticalSpeed;
            sq += e * e;
            n++;
        }
    }
    return static_cast<float>(std::sqrt(sq / n));
}

}  // namespace

// --- VarioFilter ---

TEST(VarioFilterTest, FirstSampleSeedsAltitudeOnly) {
    VarioFilter f;
    EXPECT_FALSE(f.isInitialized());
    f.update(1234.0f, 0.0f);
    EXPECT_TRUE(f.isInitialized());
    EXPECT_FLOAT_EQ(f.altitude(), 1234.0f);
    EXPECT_FLOAT_EQ(f.verticalSpeed(), 0.0f);
}

// The old filter computed (altitude - previousSpeed) / dt, so a pilot sitting
// still at 500 m read as a climb of thousands of m/s.
TEST(VarioFilterTest, StationaryAtAltitudeReadsZero) {
    VarioFilter f;
    for (int i = 0; i < 200; i++) f.update(500.0f, kDt);
    EXPECT_NEAR(f.verticalSpeed(), 0.0f, 1e-3f);
    EXPECT_NEAR(f.altitude(), 500.0f, 1e-3f);
}

TEST(VarioFilterTest, NoisyLevelFlightStaysQuiet) {
    VarioFilter f;
    Flight flight(800.0f, 0.25f);
    float rms = runAndMeasure(f, flight, 0.0f, 30.0f);
    // Quiet enough that the 0.2 m/s climb tone does not chatter in still air.
    EXPECT_LT(rms, 0.15f);
}

TEST(VarioFilterTest, TracksSteadyClimb) {
    VarioFilter f;
    Flight flight(1000.0f, 0.25f);
    float rms = runAndMeasure(f, flight, 2.0f, 30.0f);
    EXPECT_LT(rms, 0.15f);
    EXPECT_NEAR(f.altitude(), flight.altitude, 0.5f);
}

TEST(VarioFilterTest, TracksSteadySink) {
    VarioFilter f;
    Flight flight(1500.0f, 0.25f);
    float rms = runAndMeasure(f, flight, -1.2f, 30.0f);
    EXPECT_LT(rms, 0.15f);
}

TEST(VarioFilterTest, RespondsToThermalEntryWithinTwoSeconds) {
    VarioFilter f;
    Flight flight(1000.0f, 0.25f);
    for (int i = 0; i < 200; i++) f.update(flight.sample(-1.0f), kDt);  // 10 s gliding

    // Hit a 2 m/s core: 3 m/s change in vertical speed.
    float reached = 0;
    for (int i = 0; i < static_cast<int>(2.0f / kDt); i++) {
        f.update(flight.sample(2.0f), kDt);
        reached = f.verticalSpeed();
    }
    EXPECT_GT(reached, 1.6f) << "pilot should hear the thermal within 2 s";
    EXPECT_LT(reached, 2.6f) << "and not be told it is stronger than it is";
}

TEST(VarioFilterTest, LongGapRestartsFromNewSample) {
    VarioFilter f;
    for (int i = 0; i < 100; i++) f.update(1000.0f + i * 0.1f, kDt);
    EXPECT_GT(f.verticalSpeed(), 1.0f);

    f.update(900.0f, VarioFilter::kMaxGapS + 0.5f);
    EXPECT_FLOAT_EQ(f.altitude(), 900.0f);
    EXPECT_FLOAT_EQ(f.verticalSpeed(), 0.0f);
}

TEST(VarioFilterTest, NonPositiveDtRestarts) {
    VarioFilter f;
    for (int i = 0; i < 50; i++) f.update(1000.0f + i * 0.1f, kDt);
    f.update(700.0f, 0.0f);
    EXPECT_FLOAT_EQ(f.altitude(), 700.0f);
    EXPECT_FLOAT_EQ(f.verticalSpeed(), 0.0f);
}

TEST(VarioFilterTest, ResetClearsState) {
    VarioFilter f;
    f.update(1000.0f, 0.0f);
    f.reset();
    EXPECT_FALSE(f.isInitialized());
}

// --- VariometerService ---

namespace {

// Inverse of MockBarometer::calculateAltitude at the mock's 25 C default.
float pressureAt(float altitude) {
    const float t = 25.0f + 273.15f;
    return 1013.25f / std::pow(altitude * 0.0065f / t + 1.0f, 5.257f);
}

}  // namespace

class VariometerServiceTest : public ::testing::Test {
protected:
    MockBarometer baro;
    MockAudio audio;
    MockArduino arduino;
    VariometerService vario{baro, audio, arduino};

    // Steps the service through `seconds` at 20 Hz while moving at `vs`.
    void fly(float& altitude, float vs, float seconds) {
        for (int i = 0; i < static_cast<int>(seconds / kDt); i++) {
            altitude += vs * kDt;
            baro.setNextPressure(pressureAt(altitude));
            arduino.setMillis(arduino.getCurrentMillis() + static_cast<uint32_t>(kDt * 1000));
            vario.update();
        }
    }
};

TEST_F(VariometerServiceTest, NoDataBeforeFirstReading) {
    EXPECT_FALSE(vario.hasData());
}

TEST_F(VariometerServiceTest, ReportsAltitudeAndClimb) {
    float altitude = 1200.0f;
    fly(altitude, 1.5f, 10.0f);
    EXPECT_TRUE(vario.hasData());
    EXPECT_NEAR(vario.getVerticalSpeed(), 1.5f, 0.05f);
    EXPECT_NEAR(vario.getAltitude(), altitude, 0.5f);
}

TEST_F(VariometerServiceTest, ClimbBeepsAndPitchRisesWithStrength) {
    float altitude = 1000.0f;
    fly(altitude, 1.0f, 5.0f);
    ASSERT_TRUE(audio.isToneActive());
    unsigned int weak = audio.getLastTone().frequency;

    fly(altitude, 3.0f, 5.0f);
    ASSERT_TRUE(audio.isToneActive());
    EXPECT_GT(audio.getLastTone().frequency, weak);
}

TEST_F(VariometerServiceTest, NormalGlideIsSilent) {
    float altitude = 1000.0f;
    fly(altitude, -1.0f, 5.0f);
    EXPECT_FALSE(audio.isToneActive());
}

TEST_F(VariometerServiceTest, StrongSinkSoundsAlarm) {
    float altitude = 1000.0f;
    fly(altitude, -4.0f, 5.0f);
    EXPECT_TRUE(audio.isToneActive());
}

TEST_F(VariometerServiceTest, BarometerFailureDropsDataAndSilences) {
    float altitude = 1000.0f;
    fly(altitude, 2.0f, 5.0f);
    ASSERT_TRUE(vario.hasData());

    baro.setHealthStatus(false);
    arduino.setMillis(arduino.getCurrentMillis() + 50);
    vario.update();
    EXPECT_FALSE(vario.hasData());
    EXPECT_FALSE(audio.isToneActive());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS()) {}
    // Always exit 0 so PlatformIO parses the results itself.
    return 0;
}

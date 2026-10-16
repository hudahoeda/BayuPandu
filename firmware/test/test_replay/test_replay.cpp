#include <gtest/gtest.h>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include "Services/ReplayBarometer.h"
#include "Services/SimulationService.h"
#include "Services/VariometerService.h"
#include "mocks/MockArduino.h"
#include "mocks/MockAudio.h"

namespace {

// Writes an IGC file with one B-record a second. Altitude is logged in whole
// metres, as real loggers do, so a climb of 2.4 m/s shows up as 2, 3, 2, 2, 3...
std::string writeIgc(const std::string& name, int seconds, const std::function<double(int)>& altitudeAt) {
    std::filesystem::path path = std::filesystem::temp_directory_path() / name;
    std::ofstream out(path);
    out << "AXXX test flight\n";
    for (int s = 0; s <= seconds; s++) {
        int t = 12 * 3600 + s;
        int alt = static_cast<int>(std::lround(altitudeAt(s)));
        char line[64];
        std::snprintf(line, sizeof line, "B%02d%02d%02d0745123S11012345EA%05d%05d\n", t / 3600, t / 60 % 60,
                      t % 60, alt, alt + 30);
        out << line;
    }
    return path.string();
}

struct Stats {
    double mean = 0, rms = 0;  // of the vario's climb, against the expected value
};

class ReplayVarioTest : public ::testing::Test {
protected:
    MockArduino arduino;
    MockAudio audio;
    SimulationService sim{arduino};
    ReplayBarometer baro{sim};
    // The hardware barometer is never read here; the replay replaces it.
    ReplayBarometer unused{sim};
    VariometerService vario{unused, audio, arduino};

    int step = 0;  // 20 ms ticks since start()

    void start(const std::string& igc) {
        step = 0;
        arduino.setMillis(1000);
        ASSERT_TRUE(sim.initialize(igc));
        vario.setReplaySource(&baro);
    }

    // Steps the replay and the vario at 50 Hz, as the simulator's loop does, and
    // measures the vario's climb against `expected` over the last `measureS` of them.
    Stats run(float seconds, float measureS, const std::function<double(double)>& expected) {
        double sum = 0, sq = 0;
        int n = 0;
        int steps = static_cast<int>(seconds * 50);
        for (int i = 1; i <= steps; i++) {
            arduino.setMillis(1000 + ++step * 20);
            sim.update();
            vario.update();
            double t = step / 50.0;
            if (i > steps - measureS * 50) {
                double e = vario.getVerticalSpeed() - expected(t);
                sum += e;
                sq += e * e;
                n++;
            }
        }
        Stats s;
        s.mean = sum / n;
        s.rms = std::sqrt(sq / n);
        return s;
    }
};

}  // namespace

TEST(ReplayBarometerTest, PressureAndAltitudeRoundTrip) {
    MockArduino arduino;
    SimulationService sim{arduino};
    ReplayBarometer baro{sim};
    for (float alt : {0.0f, 850.0f, 2400.0f, 4500.0f}) {
        EXPECT_NEAR(baro.calculateAltitude(ReplayBarometer::pressureAt(alt)), alt, 0.05f);
    }
    float p;
    EXPECT_FALSE(baro.readPressure(p));  // nothing loaded
}

TEST_F(ReplayVarioTest, SteadyClimbReadsTrueThroughWholeMetreLogging) {
    start(writeIgc("bp-steady.igc", 90, [](int s) { return 1000.0 + 2.4 * s; }));
    Stats s = run(80.0f, 70.0f, [](double) { return 2.4; });
    EXPECT_NEAR(s.mean, 0.0, 0.05);
    // A straight line between whole-metre fixes would swing the climb by about
    // +/-0.5 m/s every second; the smooth curve plus sensor noise stays well inside.
    EXPECT_LT(s.rms, 0.2);
}

TEST_F(ReplayVarioTest, CatchesAThermalWithinTwoSeconds) {
    // Gliding at -1 m/s, then a 3 m/s core from t = 30 s.
    auto alt = [](double t) { return t < 30 ? 1500.0 - t : 1470.0 + 3.0 * (t - 30); };
    start(writeIgc("bp-thermal.igc", 60, [&](int s) { return alt(s); }));
    Stats glide = run(29.0f, 20.0f, [](double) { return -1.0; });
    EXPECT_NEAR(glide.mean, 0.0, 0.1);
    // The log's own smoothing spreads the step over about 4 s (28-32 s); by 34 s
    // the vario must read the core.
    run(5.0f, 0.0f, [](double) { return 3.0; });
    Stats core = run(10.0f, 10.0f, [](double) { return 3.0; });
    EXPECT_NEAR(core.mean, 0.0, 0.1);
    EXPECT_LT(core.rms, 0.25);
}

TEST_F(ReplayVarioTest, LostBarometerDropsData) {
    start(writeIgc("bp-lost.igc", 30, [](int s) { return 1000.0 + s; }));
    run(5.0f, 0.0f, [](double) { return 1.0; });
    ASSERT_TRUE(vario.hasData());
    sim.setBaroLost(true);
    run(0.1f, 0.0f, [](double) { return 1.0; });
    EXPECT_FALSE(vario.hasData());
}

TEST_F(ReplayVarioTest, RestartingTheReplayCarriesNoClimbAcross) {
    std::string igc = writeIgc("bp-loop.igc", 20, [](int s) { return 1000.0 + 3.0 * s; });
    start(igc);
    run(15.0f, 0.0f, [](double) { return 3.0; });
    ASSERT_GT(vario.getVerticalSpeed(), 2.5f);
    // Looping back to the start drops 45 m at once; that must not read as sink.
    start(igc);
    EXPECT_FALSE(vario.hasData());
    // The filter starts again from level flight, as it does after power-on.
    for (int i = 0; i < 5; i++) {
        run(0.2f, 0.0f, [](double) { return 3.0; });
        EXPECT_GT(vario.getVerticalSpeed(), 0.0f) << "at " << (i + 1) * 0.2 << " s";
    }
    run(2.0f, 0.0f, [](double) { return 3.0; });
    EXPECT_NEAR(vario.getVerticalSpeed(), 3.0f, 0.3f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS()) {}
    // Always exit 0 so PlatformIO parses the results itself.
    return 0;
}

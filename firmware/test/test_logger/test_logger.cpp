#include <gtest/gtest.h>
#include <string>
#include "Services/FlightLogger.h"
#include "Services/IgcFormat.h"
#include "mocks/MockStorage.h"

// --- IGC text ---

TEST(IgcFormatTest, BRecordSouthEastWithRoundingCarry) {
    igc::Fix fix;
    fix.utcMs = (10 * 3600 + 59 * 60 + 2) * 1000 + 999;  // the second, not rounded up
    fix.latitude = -7.75;                     // 7 deg 45.000' S
    fix.longitude = 110.0 + 59.99999 / 60.0;  // rounds to 111 deg 00.000' E
    fix.valid3d = true;
    fix.pressureAltitude = 1163;
    fix.gnssAltitude = 1302;
    char line[48];
    ASSERT_GT(igc::formatBRecord(line, sizeof(line), fix), 0u);
    EXPECT_STREQ(line, "B1059020745000S11100000EA0116301302\r\n");
}

TEST(IgcFormatTest, BRecordNorthWestNoFixBelowSeaLevel) {
    igc::Fix fix;
    fix.utcMs = 0;
    fix.latitude = 51.5;
    fix.longitude = -1.8417;
    fix.valid3d = false;
    fix.pressureAltitude = -12;
    fix.gnssAltitude = 0;
    char line[48];
    igc::formatBRecord(line, sizeof(line), fix);
    EXPECT_STREQ(line, "B0000005130000N00150502WV-001200000\r\n");
}

TEST(IgcFormatTest, TooSmallBufferWritesNothingUsable) {
    igc::Fix fix;
    char line[10];
    EXPECT_EQ(igc::formatBRecord(line, sizeof(line), fix), 0u);
}

TEST(IgcFormatTest, LongFileName) {
    igc::Date d;
    d.year = 2026;
    d.month = 10;
    d.day = 16;
    char name[48];
    igc::formatFileName(name, sizeof(name), "/flights", d, "SIM", 3);
    EXPECT_STREQ(name, "/flights/2026-10-16-XBP-SIM-03.igc");
}

TEST(IgcFormatTest, HeaderStartsWithARecordAndEndsInCrLf) {
    igc::PilotInfo p;
    igc::parsePilotFile("PILOT=Budi\n", p);
    igc::Header h;
    h.serial = "A1Z";
    h.date.year = 2026;
    h.date.month = 10;
    h.date.day = 16;
    h.flightOfDay = 2;
    h.pilot = &p;
    h.firmware = "0.1.0";
    h.hardware = "ESP32-S3";
    char text[768];
    ASSERT_GT(igc::formatHeader(text, sizeof(text), h), 0u);
    std::string s = text;
    EXPECT_EQ(s.rfind("AXBPA1Z", 0), 0u);
    EXPECT_NE(s.find("HFDTEDATE:161026,02\r\n"), std::string::npos);
    EXPECT_NE(s.find("HFPLTPILOTINCHARGE:Budi\r\n"), std::string::npos);
    EXPECT_NE(s.find("HFALPALTPRESSURE:ISA\r\n"), std::string::npos);
    EXPECT_EQ(s.substr(s.size() - 2), "\r\n");
}

TEST(IgcFormatTest, PilotFileTrimsCommentsAndReservedCharacters) {
    igc::PilotInfo p;
    igc::parsePilotFile("# my wing\r\n"
                        "  pilot =  Siti Rahma  \r\n"
                        "GLIDER=Gin Bonanza 3 # EN-C\r\n"
                        "Glider_ID=PK*$123!\r\n"
                        "COLOR=red\r\n",
                        p);
    EXPECT_STREQ(p.pilot, "Siti Rahma");
    EXPECT_STREQ(p.gliderType, "Gin Bonanza 3");
    EXPECT_STREQ(p.gliderId, "PK123");

    igc::PilotInfo longName;
    igc::parsePilotFile(("PILOT=" + std::string(60, 'x')).c_str(), longName);
    EXPECT_EQ(std::string(longName.pilot).size(), 40u);
}

// --- Logger ---

namespace {

class FlightLoggerTest : public ::testing::Test {
protected:
    MockStorage card;
    FlightLogger logger{card};
    uint32_t now = 0;
    uint32_t utc = 12 * 3600 * 1000;  // noon
    double lat = -7.8, lon = 110.4;

    void SetUp() override {
        logger.setDevice("T01", "test");
        card.injectFile("/pilot.txt", "PILOT=Budi\n");
    }

    // One second of flight at `speed` m/s ground speed and `climb` m/s.
    void second(float speed, float climb = 0.0f, bool fix = true) {
        now += 1000;
        utc += 1000;
        lon += speed / 111000.0;
        FlightData d;
        d.isValid = true;
        d.altitude = 900.0f;
        d.verticalSpeed = climb;
        d.gpsData.hasValidFix = fix;
        d.gpsData.latitude = lat;
        d.gpsData.longitude = lon;
        d.gpsData.altitude = 950.0f;
        d.gpsData.speed = speed;
        d.gpsData.timestamp = utc;
        d.gpsData.year = 2026;
        d.gpsData.month = 10;
        d.gpsData.day = 16;
        logger.update(d, now);
    }
    void seconds(int n, float speed, float climb = 0.0f, bool fix = true) {
        for (int i = 0; i < n; i++) second(speed, climb, fix);
    }

    std::string file(const char* name) {
        auto it = card.getFiles().find(name);
        return it == card.getFiles().end() ? "" : it->second;
    }
    static int records(const std::string& text) {
        int n = 0;
        for (size_t i = 0; i < text.size(); i++) n += text[i] == 'B' && (i == 0 || text[i - 1] == '\n');
        return n;
    }
};

const char* kFirst = "/flights/2026-10-16-XBP-T01-01.igc";
const char* kSecond = "/flights/2026-10-16-XBP-T01-02.igc";

}  // namespace

TEST_F(FlightLoggerTest, ReadyOnTheGroundWithoutWritingAnything) {
    ASSERT_TRUE(logger.initialize());
    EXPECT_TRUE(card.hasDir("/flights"));
    seconds(300, 1.0f);  // walking about at launch
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::Ready);
    EXPECT_EQ(card.getWriteFileCallCount(), 0);
    EXPECT_EQ(card.getAppendFileCallCount(), 0);
}

TEST_F(FlightLoggerTest, TakeoffOpensFileWithTheMinuteBeforeIt) {
    logger.initialize();
    seconds(120, 1.0f);
    seconds(FlightLogger::kTakeoffHoldS, 8.0f);
    ASSERT_EQ(logger.getStatus(), FlightLogger::Status::Recording);
    EXPECT_STREQ(logger.getFileName(), kFirst);
    std::string text = file(kFirst);
    EXPECT_NE(text.find("HFPLTPILOTINCHARGE:Budi"), std::string::npos);
    // The last minute on the ground, ending with the fix that confirmed takeoff.
    EXPECT_EQ(records(text), FlightLogger::kPreTakeoffS);
    EXPECT_NE(text.find("B120205"), std::string::npos);   // 125 s after noon
    EXPECT_EQ(text.find("B120104"), std::string::npos);   // 64 s: more than a minute before

    seconds(30, 9.0f, 1.5f);
    EXPECT_EQ(records(file(kFirst)), FlightLogger::kPreTakeoffS + 30);
    EXPECT_EQ(logger.getRecordingSeconds(now), 30u);
}

TEST_F(FlightLoggerTest, LandingClosesAfterAStillMinuteAndNextFlightIsNumberTwo) {
    logger.initialize();
    seconds(10, 8.0f);
    seconds(600, 9.0f, -1.0f);
    seconds(FlightLogger::kLandingS - 1, 0.5f);
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::Recording);
    second(0.5f);
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::Ready);

    seconds(20, 7.0f);
    EXPECT_STREQ(logger.getFileName(), kSecond);
}

TEST_F(FlightLoggerTest, HoveringInLiftIsNotALanding) {
    logger.initialize();
    seconds(10, 8.0f);
    seconds(3 * FlightLogger::kLandingS, 0.2f, 1.2f);  // soaring a ridge into the wind
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::Recording);
}

TEST_F(FlightLoggerTest, LostFixKeepsLoggingAtTheLastPosition) {
    logger.initialize();
    seconds(10, 8.0f);
    second(8.0f);
    std::string before = file(kFirst);
    std::string lastPos = before.substr(before.rfind('B') + 7, 17);
    seconds(3, 8.0f, 0.0f, false);
    std::string text = file(kFirst);
    std::string last = text.substr(text.rfind('B'));
    EXPECT_EQ(last.substr(7, 17), lastPos);
    EXPECT_EQ(last[24], 'V');
    EXPECT_EQ(last.substr(30, 5), "00000");  // no GNSS altitude without a fix
}

TEST_F(FlightLoggerTest, NoCardMeansNoLogAndSaysSo) {
    card.setHealthStatus(false);
    EXPECT_FALSE(logger.initialize());
    seconds(30, 9.0f);
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::NoCard);
    EXPECT_TRUE(card.getFiles().size() == 1);  // only pilot.txt
}

TEST_F(FlightLoggerTest, CardFailingInFlightIsReported) {
    logger.initialize();
    seconds(10, 8.0f);
    ASSERT_EQ(logger.getStatus(), FlightLogger::Status::Recording);
    card.setHealthStatus(false);
    second(9.0f);
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::WriteFailed);
    card.setHealthStatus(true);
    second(9.0f);
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::Recording);
}

TEST_F(FlightLoggerTest, EndFlightStartsAFreshFile) {
    logger.initialize();
    seconds(10, 8.0f);
    logger.endFlight();
    EXPECT_EQ(logger.getStatus(), FlightLogger::Status::Ready);
    seconds(FlightLogger::kTakeoffHoldS, 8.0f);
    EXPECT_STREQ(logger.getFileName(), kSecond);
    EXPECT_EQ(records(file(kSecond)), FlightLogger::kTakeoffHoldS);  // the old buffer is gone
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS()) {}
    // Always exit 0 so PlatformIO parses the results itself.
    return 0;
}

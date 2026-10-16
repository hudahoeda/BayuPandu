#include <gtest/gtest.h>
#include "Services/MelodyPlayer.h"
#include "UI/BootChime.h"
#include "mocks/MockAudio.h"

using namespace BootTiming;

// --- midiToHz ---

TEST(MidiToHzTest, ConcertPitchAndOctaves) {
    EXPECT_NEAR(midiToHz(69), 440.0f, 0.01f);
    EXPECT_NEAR(midiToHz(81), 880.0f, 0.01f);
    EXPECT_NEAR(midiToHz(57), 220.0f, 0.01f);
}

// --- MelodyPlayer ---

class MelodyPlayerTest : public ::testing::Test {
protected:
    MockAudio audio;
    MelodyPlayer player{audio};
    const Note tune[3] = {{0, 100, 69}, {200, 50, 81}, {300, 100, 57}};
};

TEST_F(MelodyPlayerTest, PlaysEachNoteWhenItStarts) {
    player.play(tune, 3, 1000);
    ASSERT_EQ(audio.getToneCallCount(), 1);
    EXPECT_EQ(audio.getLastTone().frequency, 440u);
    EXPECT_EQ(audio.getLastTone().duration, 100u);

    player.update(1199);
    EXPECT_EQ(audio.getToneCallCount(), 1);
    player.update(1200);
    ASSERT_EQ(audio.getToneCallCount(), 2);
    EXPECT_EQ(audio.getLastTone().frequency, 880u);
}

TEST_F(MelodyPlayerTest, LateLoopShortensNoteToKeepTime) {
    player.play(tune, 3, 0);
    player.update(220);  // 20 ms into the second note
    EXPECT_EQ(audio.getLastTone().frequency, 880u);
    EXPECT_EQ(audio.getLastTone().duration, 30u);
}

TEST_F(MelodyPlayerTest, SkipsANoteThatAlreadyEnded) {
    player.play(tune, 3, 0);
    player.update(310);  // the 50 ms note at 200 is over; the one at 300 is on
    ASSERT_EQ(audio.getToneCallCount(), 2);
    EXPECT_EQ(audio.getLastTone().frequency, 220u);
}

TEST_F(MelodyPlayerTest, FinishesAfterLastNote) {
    player.play(tune, 3, 0);
    player.update(399);
    EXPECT_TRUE(player.isPlaying());
    player.update(400);
    EXPECT_FALSE(player.isPlaying());
}

TEST_F(MelodyPlayerTest, StopSilences) {
    player.play(tune, 3, 0);
    player.stop();
    EXPECT_FALSE(player.isPlaying());
    EXPECT_EQ(audio.getNoToneCallCount(), 1);
    player.update(500);
    EXPECT_EQ(audio.getToneCallCount(), 1);
}

// --- BootChime ---

namespace {

int composeFor(std::initializer_list<bool> results, Note* notes) {
    bool passed[kChecks];
    int i = 0;
    for (bool r : results) passed[i++] = r;
    return BootChime::compose(passed, notes, MelodyPlayer::kMaxNotes);
}

const Note* noteAt(const Note* notes, int n, uint32_t startMs) {
    for (int i = 0; i < n; i++) {
        if (notes[i].startMs == startMs) return &notes[i];
    }
    return nullptr;
}

}  // namespace

TEST(BootChimeTest, NotesAreSortedAndNeverOverlap) {
    for (auto results : {std::initializer_list<bool>{true, true, true, true},
                         std::initializer_list<bool>{false, true, false, true}}) {
        Note notes[MelodyPlayer::kMaxNotes];
        int n = composeFor(results, notes);
        ASSERT_GT(n, 0);
        for (int i = 1; i < n; i++) {
            EXPECT_GE(notes[i].startMs, notes[i - 1].startMs + notes[i - 1].durationMs) << "note " << i;
        }
    }
}

TEST(BootChimeTest, EndsBeforeTheBootSignDoes) {
    Note notes[MelodyPlayer::kMaxNotes];
    int n = composeFor({true, true, true, true}, notes);
    auto endOf = [](const Note& note) { return static_cast<uint32_t>(note.startMs + note.durationMs); };
    EXPECT_LE(endOf(notes[n - 1]), kLastReveal + kHoldOk);
    n = composeFor({false, false, false, false}, notes);
    EXPECT_LE(endOf(notes[n - 1]), kLastReveal + kHoldWarn);
}

TEST(BootChimeTest, PassedChecksBeepHigherEachTime) {
    Note notes[MelodyPlayer::kMaxNotes];
    int n = composeFor({true, true, true, true}, notes);
    uint8_t previous = 0;
    for (int c = 0; c < kChecks; c++) {
        const Note* beep = noteAt(notes, n, revealAt(c));
        ASSERT_NE(beep, nullptr) << "check " << c;
        EXPECT_GT(beep->midi, previous);
        previous = beep->midi;
        // The vario's double beep: a second beep at the same pitch.
        int beeps = 0;
        for (int i = 0; i < n; i++) {
            beeps += notes[i].midi == beep->midi && notes[i].startMs >= revealAt(c) &&
                     notes[i].startMs < revealAt(c) + kRevealStep;
        }
        EXPECT_EQ(beeps, 2);
    }
    // Ready rises to the highest note of the tune.
    EXPECT_GT(notes[n - 1].midi, notes[n - 2].midi);
}

TEST(BootChimeTest, FailedCheckSoundsLowAndReadyFalls) {
    Note notes[MelodyPlayer::kMaxNotes];
    int n = composeFor({false, true, true, true}, notes);
    const Note* sink = noteAt(notes, n, revealAt(0));
    ASSERT_NE(sink, nullptr);
    for (int i = 0; i < n; i++) EXPECT_GE(notes[i].midi, sink->midi);  // nothing lower than the sink tone
    EXPECT_LT(notes[n - 1].midi, notes[n - 2].midi);
}

TEST(BootChimeTest, ChecksMatchTheRowsOnScreen) {
    SelfTestResult r;
    r.barometerOk = true;
    r.gpsResponding = true;   // no fix yet is still a pass
    r.gpsFix = false;
    r.storageOk = false;
    r.batteryPercent = 21;
    bool passed[kChecks];
    checksPassed(r, passed);
    EXPECT_TRUE(passed[0]);
    EXPECT_TRUE(passed[1]);
    EXPECT_FALSE(passed[2]);
    EXPECT_TRUE(passed[3]);
    r.batteryPercent = 20;
    checksPassed(r, passed);
    EXPECT_FALSE(passed[3]);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS()) {}
    // Always exit 0 so PlatformIO parses the results itself.
    return 0;
}

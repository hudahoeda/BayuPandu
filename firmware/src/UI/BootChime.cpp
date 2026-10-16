#include "BootChime.h"

namespace BootChime {

namespace {

using namespace BootTiming;

// D major pentatonic, set high: a piezo buzzer is loudest and cleanest from about
// 1 to 3 kHz, the same range a vario climbs through.
constexpr uint8_t kD5 = 74, kE5 = 76, kFs5 = 78, kA5 = 81, kB5 = 83;
constexpr uint8_t kD6 = 86, kE6 = 88, kFs6 = 90, kA6 = 93, kD7 = 98;
constexpr uint8_t kA4 = 69;  // the sink tone

constexpr uint8_t kWindRun[] = {kD5, kE5, kFs5, kA5, kB5};
constexpr uint16_t kRunNoteMs = 40, kRunStepMs = 50;

constexpr uint8_t kCheckNote[kChecks] = {kD6, kE6, kFs6, kA6};
constexpr uint16_t kBeepMs = 70, kBeepGapMs = 40;   // vario climb beep: short, short
constexpr uint16_t kSinkMs = 260;

constexpr uint32_t kReadyAt = kLastReveal + 400;

}  // namespace

int compose(const bool passed[kChecks], Note* out, int maxNotes) {
    int n = 0;
    auto add = [&](uint32_t start, uint16_t duration, uint8_t midi) {
        if (n < maxNotes) out[n++] = {static_cast<uint16_t>(start), duration, midi};
    };

    // The wind arrives with the wing.
    for (unsigned i = 0; i < sizeof(kWindRun); i++) add(kWingAppear + i * kRunStepMs, kRunNoteMs, kWindRun[i]);

    bool allPassed = true;
    for (int c = 0; c < kChecks; c++) {
        uint32_t at = revealAt(c);
        if (passed[c]) {
            add(at, kBeepMs, kCheckNote[c]);
            add(at + kBeepMs + kBeepGapMs, kBeepMs, kCheckNote[c]);
        } else {
            add(at, kSinkMs, kA4);
            allPassed = false;
        }
    }

    if (allPassed) {
        add(kReadyAt, 80, kA6);
        add(kReadyAt + 100, 220, kD7);
    } else {
        add(kReadyAt, 120, kFs5);
        add(kReadyAt + 140, 300, kD5);
    }
    return n;
}

}  // namespace BootChime

#pragma once

#include "Data/Types.h"
#include <cstdint>

// When things happen on the boot sign, in ms from its first frame. The animation
// (BootScreen) and the startup chime (BootChime) both read these, so the sound
// lands on the picture.
namespace BootTiming {

constexpr int kChecks = 4;                 // BARO, GPS, CARD, BATT, in that order

constexpr uint32_t kWindStart = 80;        // first wind line starts drawing
constexpr uint32_t kWindDraw = 520;        // time for one line to cross the sign
constexpr uint32_t kWindStagger = 90;      // each line starts this much after the last
constexpr uint32_t kWingAppear = 300;      // wing appears low over the crest as the wind comes in
constexpr uint32_t kFirstReveal = 900;     // first check result shows
constexpr uint32_t kRevealStep = 300;      // then one result every step
constexpr uint32_t kLiftMs = 250;          // the wing's rise after each result
constexpr uint32_t kHoldOk = 900;          // hold after the last result when all passed
constexpr uint32_t kHoldWarn = 2200;       // longer when something failed
constexpr uint32_t kLastReveal = kFirstReveal + (kChecks - 1) * kRevealStep;

constexpr uint32_t revealAt(int check) { return kFirstReveal + check * kRevealStep; }

// Which checks passed. The one rule for both the yellow rows and the chime's low
// notes: no fix yet is normal at switch-on, so only a silent GPS module fails.
inline void checksPassed(const SelfTestResult& r, bool passed[kChecks]) {
    passed[0] = r.barometerOk;
    passed[1] = r.gpsResponding;
    passed[2] = r.storageOk;
    passed[3] = r.batteryPercent > 20;
}

}  // namespace BootTiming

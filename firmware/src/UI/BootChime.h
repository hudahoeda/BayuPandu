#pragma once

#include "Services/MelodyPlayer.h"
#include "UI/BootTiming.h"

// The startup chime, timed to the boot sign (BootTiming). A quick rising run as
// the wind comes in; then for each check, the vario's climb double-beep a step
// higher if it passed, or a low sink tone if it failed; then a bright rising
// "ready", or a calm falling pair when anything failed.
namespace BootChime {

// Writes the chime for these check results into `out` and returns the note count.
int compose(const bool passed[BootTiming::kChecks], Note* out, int maxNotes);

}  // namespace BootChime

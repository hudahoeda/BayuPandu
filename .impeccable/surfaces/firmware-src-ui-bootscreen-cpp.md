---
version: 1
slug: "firmware-src-ui-bootscreen-cpp"
primary_target: "firmware/src/UI/BootScreen.cpp"
related_targets: ["firmware/src/UI/BrandMark.cpp","assets/brand"]
---

# Boot screen and brand mark

Scope: the startup sequence on the 240x320 TFT (about 2.5 s, any button skips) and the BayuPandu mark it introduces, which also ships as SVG for the README, a one-colour sticker/case version and a favicon. Mode: Experience.
Audience/job: a pilot switching the instrument on at launch, wanting to know it is alive and its sensors answered; secondary, anyone meeting the project on GitHub.
Constraints: inherits DESIGN.md (sign green, legend white, yellow only for trouble, Overpass, flat). Mark is vector: polygons drawn by LVGL, never a bitmap. Heap: boot objects freed before the flight pages are used. Checks: BARO, GPS (module answers, not fix), LOG (storage), BATT.
Must not feel like: a phone splash, a progress spinner, or a picture pasted on a sign.

## Direction contract
THESIS: Bayu made visible: wind runs in low, meets a ridge and turns upward, and the wing rides that lift. Refuses the logo-centred splash with a progress bar under it.
OWN-WORLD: The road-sign panel of DESIGN.md: green ground, 3 px white border, white Overpass capitals, flat white sign symbols with round-capped strokes. One white ridge line, three white wind lines with arrowheads. The mark is a flat front-view wing (an arc of cells) on three short wind lines. Yellow only for a failed check.
STORY: The pilot sees the name, sees wind lift the wing up the sign as each check answers, and lands on Flight knowing what passed; a failed check is a yellow line they cannot miss.
FIRST VIEWPORT: BAYUPANDU large top-left with V0.1.0 under it. A ridge line across the middle, rising left to a crest right of centre. Three wind lines enter from the left edge, follow the slope and curve up with arrowheads under the wing mark at upper right. Below the ridge, four check rows (key left, result right, 22 px). Caption bottom: ANY BUTTON TO SKIP.
FORM: Ridge Lift, candidate 7 of my ordered surface list (re-roll 1), seed key 8b1bafee. Approved comp: .impeccable/mocks/decision/r1/ridge-lift.png.
SIGNATURE MOTION: wind lines draw in from the left along the slope; the wing starts low at the crest and rises one step per passed check; then the panel cuts to Flight.
FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

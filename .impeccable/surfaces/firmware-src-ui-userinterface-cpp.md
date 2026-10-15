---
version: 1
slug: "firmware-src-ui-userinterface-cpp"
primary_target: "firmware/src/UI/UserInterface.cpp"
related_targets: ["firmware/src/UI"]
---

# In-flight UI (240×320 TFT)

Scope: every on-device screen: Flight (main), Thermal, Navigation, Status, Settings, plus warning states. Mode: Operate.
Audience/job: an XC paraglider pilot glancing down at a cockpit pod (50–70 cm away) in tropical sun, needing climb, altitude, ground speed and L/D in about 1 s.
Constraints: LVGL 8.3 on ESP32, 4 buttons (UP/DOWN cycle pages, ENTER, CANCEL), EN/ID labels, no colour-only meaning, a fixed page set.
Must not feel like: a phone app, a busy screen, or a generic grid of boxes.

## Direction contract
THESIS: Every figure is set as a highway direction sign legend, readable at speed in one glance. Refuses the category default of a box grid with a side vario bar.
OWN-WORLD: Retroreflective sign green #00613F ground, white #FFFFFF legends and 3 px white sign border with rounded corners, warning yellow #FFC20E panels with black legends and a diamond glyph, black #111111 for ink on yellow. Overpass (Highway Gothic lineage) heavy and black weights; no boxes, cards, gradients or shadows; a white rule separates the sign's sections.
STORY: The pilot reads climb first, then height, then whether the glide works. Warnings replace a section as a yellow sign, so they're impossible to miss and never mistaken for data.
FIRST VIEWPORT: Status strip on top (page name, GPS sats, flight time, battery). Climb rate is the hero: a sign arrow (up when climbing, down when sinking, flat when level) plus an ~84 px number and m/s. A white rule, then ALT, GS and L/D as sign "distance lines": key left, value right, 38 px. A bottom slot holds avg climb, or a yellow warning sign.
FORM: Direction Sign, candidate 1 of my ordered list (IMPECCABLE'S PICK), seed key 4ba3dd3f.
SIGNATURE: The climb arrow is the sign's direction arrow. Its direction and size encode sign and strength, and it flips with the climb, so lift and sink differ in shape, not just colour.
FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

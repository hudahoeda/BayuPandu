---
name: BayuPandu
description: In-flight UI of an open paragliding flight computer, set as a highway direction sign on a 240x320 TFT.
colors:
  sign-green: "#00613F"
  legend-white: "#FFFFFF"
  warning-yellow: "#FFC20E"
  warning-ink: "#111111"
  surround: "#0E1A14"
typography:
  display:
    fontFamily: "Overpass Black (LVGL bitmap bp_figures_84, 4 bpp)"
    fontSize: "84px"
    fontWeight: 900
  headline:
    fontFamily: "Overpass Black (LVGL bitmap bp_figures_38, 4 bpp)"
    fontSize: "38px"
    fontWeight: 900
  title:
    fontFamily: "Overpass ExtraBold (LVGL bitmap bp_text_22, 4 bpp)"
    fontSize: "22px"
    fontWeight: 800
  label:
    fontFamily: "Overpass ExtraBold (LVGL bitmap bp_text_14, 4 bpp)"
    fontSize: "14px"
    fontWeight: 800
    letterSpacing: "1px"
  caption:
    fontFamily: "Overpass ExtraBold (LVGL bitmap bp_text_11, 4 bpp)"
    fontSize: "11px"
    fontWeight: 800
  wordmark:
    fontFamily: "Overpass Black (LVGL bitmap bp_wordmark_32, 4 bpp)"
    fontSize: "32px"
    fontWeight: 900
    letterSpacing: "-1px"
  version:
    fontFamily: "Overpass Black (LVGL bitmap bp_version_16, 4 bpp)"
    fontSize: "16px"
    fontWeight: 900
rounded:
  band: "3px"
  panel: "8px"
spacing:
  panel-margin: "4px"
  inset: "6px"
  hero-gap: "8px"
  unit-column: "46px"
  strip: "26px"
  footer: "22px"
  row: "44px"
  check-row: "23px"
components:
  sign-panel:
    backgroundColor: "{colors.sign-green}"
    textColor: "{colors.legend-white}"
    rounded: "{rounded.panel}"
    width: "232px"
    height: "312px"
  sign-panel-warning:
    backgroundColor: "{colors.warning-yellow}"
    textColor: "{colors.warning-ink}"
    rounded: "{rounded.panel}"
    width: "232px"
    height: "312px"
  hero-figure:
    textColor: "{colors.legend-white}"
    typography: "{typography.display}"
  distance-line:
    textColor: "{colors.legend-white}"
    typography: "{typography.headline}"
    height: "{spacing.row}"
  footer-warning:
    backgroundColor: "{colors.warning-yellow}"
    textColor: "{colors.warning-ink}"
    typography: "{typography.label}"
    rounded: "{rounded.band}"
    height: "{spacing.footer}"
  setting-row:
    textColor: "{colors.legend-white}"
    typography: "{typography.title}"
    height: "50px"
  setting-row-focused:
    backgroundColor: "{colors.legend-white}"
    textColor: "{colors.sign-green}"
    typography: "{typography.title}"
    rounded: "{rounded.band}"
    height: "50px"
  boot-wordmark:
    textColor: "{colors.legend-white}"
    typography: "{typography.wordmark}"
  boot-check-row:
    textColor: "{colors.legend-white}"
    typography: "{typography.title}"
    height: "{spacing.check-row}"
  boot-check-row-failed:
    backgroundColor: "{colors.warning-yellow}"
    textColor: "{colors.warning-ink}"
    typography: "{typography.title}"
    rounded: "{rounded.band}"
    width: "220px"
    height: "26px"
---

# Design System: BayuPandu

## Overview

**Creative North Star: "The Direction Sign"**

Every page is one highway guide sign: a retroreflective green panel with a white border, white legends in a road-sign face, and a single white rule dividing the sign into sections. Figures are set as sign legends, big and heavy, so climb, height and glide read in about one second from a cockpit pod in direct sun. The system refuses the instrument-category default of a grid of boxes with a side vario bar.

Density is low by design. Each page has one hero (a sign glyph beside an 84 px figure) and a short stack of distance lines below it. Meaning is carried by shape and position as well as colour: lift and sink are different arrows, warnings are a different sign. There is no depth and no ornament. Motion is values changing on a 200 ms refresh, with one exception: the boot sign, where wind draws in along a ridge and the wing mark rises one step for each self-test that passes, then the panel cuts to Flight.

The palette has one job per colour. Green is the ground, white is every legend, yellow means something is wrong and nothing else.

**Key Characteristics:**
- One road-sign panel per page, 3 px white border, 8 px corners, on a near-black surround.
- Hero row: polygon sign glyph plus 84 px Overpass Black figure, magnitude only.
- Distance lines: key left, 38 px figure right, unit in its own column.
- Yellow is reserved for warnings and the error sign.
- Unknown values are a single dash with the glyph hidden.
- Flat: no cards, shadows, gradients or colour-only meaning.
- One mark: a front-view wing of nine cells over three wind lines, always one colour.
- One authored motion: the boot sign's ridge lift; everything else changes by value only.

## Colors

A road-sign palette: one saturated ground, one legend colour, one warning colour and its ink.

### Primary
- **Retroreflective Sign Green** (`kSignGreen`): the ground of every normal page panel, and the text colour of a focused (inverted) settings row.

### Secondary
- **Warning Sign Yellow** (`kWarning`): only for warnings. Fills the footer when a warning is up, the band behind a failed check on the boot sign, and the whole panel on the error page. Never used for data, focus, decoration or the brand mark.

All five colours live in `firmware/src/UI/SignPalette.h`, shared by every screen; `assets/brand/gen_brand.py` repeats the same hex values for the SVG files.

### Neutral
- **Legend White** (`kLegend`): every legend, figure, glyph, the panel border, the section rule, and the band of a focused settings row.
- **Warning Ink** (`kInk`): legends, border and the diamond glyph on yellow. Appears only on yellow.
- **Cab Surround** (`kSurround`): the 4 px screen margin around the sign panel, a green-tinted near-black so the panel reads as a mounted sign rather than a full-bleed screen. Also the ground of the social-preview card.

### Named Rules
**The Yellow Means Trouble Rule.** Warning yellow appears only when something is wrong: a footer warning sign, a failed boot check, or the error panel. If a surface is yellow and nothing is wrong, it is a bug.

**The Shape Carries Meaning Rule.** Every colour signal has a shape twin: the footer warning adds the diamond glyph, a failed boot check adds the diamond and costs the wing its lift, sink is a down arrow and not a red number, level is a bar. The UI must read correctly on a monochrome panel.

## Typography

**Display Font:** Overpass Black (wght 900), figures only
**Body Font:** Overpass ExtraBold (wght 800)

**Character:** Overpass descends from US Highway Gothic, so the legends are literally road-sign lettering. Black for figures and the name, ExtraBold for words; no lighter weights exist on the device.

Fonts are 4 bpp LVGL bitmaps generated by `firmware/tools/gen_fonts.sh` at fixed sizes with limited glyph ranges. A size or glyph not in that script does not exist on the device.

### Hierarchy
- **Display** (Black, 84 px, `bp_figures_84`, glyphs `-./0-9` only): the hero figure on Flight, Thermal and Navigation.
- **Headline** (Black, 38 px, `bp_figures_38`, digits, sign, `.:%`, `k`, `m`, degree): distance-line figures, and the fallback for a hero figure too wide for its room.
- **Title** (ExtraBold, 22 px, `bp_text_22`): two-line row values on Status and Settings, the Navigation destination legend, the error headline.
- **Label** (ExtraBold, 14 px, `bp_text_14`, 1 px tracking on page titles and keys, upper case): page title, distance-line keys and units, hero units, footer data and warnings, error advice.
- **Caption** (ExtraBold, 11 px, `bp_text_11`): the status strip, footer button hints, the Navigation position key, the Settings version line (VERSION 0.1.0), and the boot skip caption (1 px tracking, centred).
- **Wordmark** (Black, 32 px, `bp_wordmark_32`, glyphs of BAYUPANDU only: A B D N P U Y, -1 px tracking): the name on the boot sign. The SVG lockups set the same face at the same relative tracking (-1/32 em) from outlines.
- **Version** (Black, 16 px, `bp_version_16`, glyphs `V 0-9 .` only): the version line under the boot wordmark, printed as V plus `BAYUPANDU_VERSION` from `firmware/src/version.h`.

### Named Rules
**The Generated Sizes Rule.** Only the seven generated fonts exist; the wordmark and version fonts carry only the glyphs of their one string. A new size or glyph means editing `gen_fonts.sh` and paying for it in flash; never approximate with scaling.

**The Cap-Line Placement Rule.** On the boot sign, legends are positioned by the top of their capitals, not their line box: Overpass capitals are 0.70 em tall, so a label's top sits (line height - base line - 0.70 em) above the cap line it is set to.

**The All-Caps Legend Rule.** Every word on the sign is upper case, EN and ID alike, in short aviation abbreviations (ALT, GS, L/D).

## Layout

Fixed 240x320 portrait canvas. The sign panel is 232x312 at (4,4) with a 3 px border, leaving a 226 px content width; legends sit at a 6 px inset (214 px inner width).

Vertical structure of a data page, top to bottom:
- **Status strip** (26 px): page title left at 14 px; the strip right-aligned in 11 px, its width clipped to whatever the title leaves (minus 10 px), so the two can never collide. Contents: SIM, satellites or GPS state, flight time when airborne, battery percent.
- **Hero** (to y 136): glyph box 48 px wide at x 8; figure starts 8 px after it (x 64); unit tucked under the figure's right end at y 114, just above the rule.
- **Rule** (3 px white, y 136): the one section divider.
- **Distance lines** (44 px each from y 142): key left, figure right with a 46 px unit column, shared baseline.
- **Footer** (22 px at y 280): data or button hint at rest, warning sign when a warning is up.

List pages (Status, Settings) put the rule directly under the strip (y 28) and stack two-line rows: key above, value below, so long values never collide. Status rows are 44 px on a 48 px pitch; Settings rows are 50 px on a 56 px pitch.

The boot sign is the one page without a status strip or white rule. Its structure, top to bottom: wordmark (cap top y 19, x 16) and version (cap top y 51, x 17) at top left; an art box (224x146 at 8,46) holding the ridge, wind lines and wing mark; four check rows with cap tops at y 197, 220, 243, 266 (23 px pitch); the skip caption centred with its cap top at y 298. The ridge is the boot sign's divider.

Check-row columns sit at x 51 (key) and x 142 (value) unless the language's widest possible word needs room; then they move left just enough, keeping values inside x 229 and at least 20 px between the columns. Widths are measured against every result the language could show, so a failing boot lays out like a passing one.

### Named Rules
**The One Sign Rule.** A page is exactly one sign panel. Sections are divided by the white rule, never by nested boxes.

**The Clip, Don't Collide Rule.** When two legends share a line, the lower-priority one gets the remaining width and clips. Hero figures too wide for their room drop to 38 px rather than run into the unit.

## Elevation & Depth

None. The system is flat: one opaque panel on a dark surround, solid fills, no shadows, gradients or transparency. Hierarchy comes from size (84 / 38 / 22 / 14 / 11 px), position, and the white rule. Stroke edges on the boot art are anti-aliased by coverage; that is edge quality, not transparency.

### Named Rules
**The Flat Sign Rule.** A road sign has no depth. No shadows, gradients, glows or cards, on any page.

## Shapes

Rounded rectangle panel (8 px) with a 3 px border in the legend colour (white on green, ink on yellow). Inner bands (footer warning, focused row) use 3 px corners. Glyphs are filled polygons drawn by `SignGlyph`:
- **Arrow:** a highway-sign arrow, head 92% of the glyph span wide and 62% tall, shaft 36% wide. The head is fixed so it always reads as an arrow; the shaft grows with strength (15% to 100% of the remaining reach). Rotates to any bearing.
- **Level bar:** 92% x 30% of the span, for level flight (|climb| below 0.2 m/s).
- **Diamond:** the warning sign, 14x14 in the footer, 12x12 on a failed boot check.

Line work (the boot ridge and wind lines) is a stroke of constant weight with round ends, drawn as a smooth Catmull-Rom curve through traced points, with a small solid sign arrowhead at the tip of each wind line.

### Brand Mark
A paraglider wing seen from the front, over three short wind lines; defined for a mark 1 unit wide, origin at the wing's top centre, in `firmware/src/UI/BrandMark.cpp` and repeated in `assets/brand/gen_brand.py` (change both together).
- **Wing:** 9 cells on an arc spanning 42 degrees each side of vertical, outer radius set so the chord is 1 unit (0.747), 1.8 degree gaps between cells. Cell depth tapers from 0.18 at the centre to 0.11 at the tips (quadratic), and each tip is rounded with a disc as deep as the tip cell.
- **Wind lines:** three horizontal strokes 0.034 thick with round caps, at y 0.345 (x -0.18 to 0.22), 0.410 (x -0.17 to 0.21) and 0.475 (x -0.07 to 0.11). The whole mark is 0.492 units tall.
- **Drawn, never imaged:** on the device the mark is LVGL polygons, discs and lines; in the brand files it is SVG geometry. No raster source exists.
- **Lockups:** mark left, BAYUPANDU right in Overpass Black, the mark about 2 em wide (190 at a 96 px wordmark) with a 0.42 em gap, centred on the cap height. The sign-panel lockup sits on a green panel with a white border (8 at 220 tall, 22 corner radius); the bare black (ink) and white lockups keep a margin of 0.11 mark-widths so the rounded tips clear the edge.
- **Tile:** square green sign, corner radius 16% of the side, white border 3.5%, mark 68% wide and centred (512 and 180 px PNGs).
- **Favicon:** at 32 px and up (`favicon.svg`) a borderless tile with a simplified mark 82% wide: 5 cells with 6 degree gaps over 2 thicker wind lines. At 16 px the mark is a hand-fitted pixel map in `gen_brand.py`: five cells stepping down from the centre with one-pixel notches, over two wind lines, rounded corners transparent.


## Components

### Sign Panel
The page itself. Green ground, white border and legends; the error variant is the whole panel in yellow with ink border and legends and no title, because its own legend names the problem.

### Hero Row (signature)
The climb arrow is the sign's direction arrow. On Flight and Thermal: up arrow for lift, down for sink, level bar when level, shaft length = |climb| / 4 m/s (clamped). On Navigation: the arrow points to takeoff relative to the current track (up means ahead), at full length, in a square 48x48 box so it can turn freely; it hides within 100 m of takeoff. The figure prints the magnitude only; the arrow carries the sign. Thermal adds a 14 px caption top-right (CLIMBING / LAST THERMAL / NO THERMAL YET); Navigation adds the 22 px destination legend top-right, in guide-sign order: destination, arrow, distance.

### Distance Line
Key left in 14 px tracked caps, figure right in 38 px Black ending 46 px from the edge, unit (14 px) in that column, all on a shared baseline. Units-less values (L/D, track, time) leave the column empty rather than shifting the figure.

### Footer
At rest: a 14 px data line on Flight (average climb) or an 11 px button hint elsewhere, in white on green. With a warning: the band fills yellow, the diamond appears at left, and the message is 14 px ink. Warning priority: battery critical, no sensor data, no GPS fix, low battery, then transient alerts.

### Setting Row
Two-line row (key above, 22 px value below). In edit mode the focused row inverts like a selected sign legend: white band, green text. Unfocused rows stay white on green.

### Boot Sign (signature)
The startup page and the only screen that animates. A standard sign panel carrying the wordmark and version top left, the ridge-lift art, four self-test rows and a skip caption.
- **Check rows:** BARO, GPS, CARD, BATT (BARO, GPS, KARTU, BAT in Indonesian), key and value both 22 px. GPS passes when the module answers; no fix yet is normal and shows NO FIX or the satellite count. Battery fails at 20% or below.
- **Failed check:** the row gains a yellow band (220x26 at x 10, 3 px corners, starting 5 px above the cap line), the 12 px ink diamond at its left, and ink legends. It also gives the wing no lift for that row.
- **Art:** a 2.9 px white ridge line rising left to a crest right of centre, three 2.3 px wind lines entering from the left edge, following the slope and curving up under the wing, each ending in an arrowhead; the brand mark 84 px wide, centred at x 181, above the crest.
- **Heap:** the boot sign owns one LVGL screen; it is built after setup and deleted before the flight pages are built, so the two never coexist.

**The Lift Is Earned Rule.** The wing rises one equal step (20 px over four checks) only for a check that passed. A failure leaves it lower; it is never lifted to the top to look finished.

**The Wind Follows The Wing Rule.** The last 15% of each wind line is pushed by the same lift as the wing, so the arrowheads stay under the wing instead of running into it.

**The Exact Stroke Rule.** Curved strokes on the boot art are rasterised in `BootScreen.cpp` with exact-distance coverage (each pixel takes the stroke colour in proportion to its distance from the nearest segment), because LVGL 8.3 places polygon corners on whole pixels and leaves 1 px jogs and uneven weight on gentle curves. Use LVGL polygons only for solid shapes (arrowheads, glyphs, wing cells).

#### Motion grammar
The boot sign is the system's only authored motion; times are from the first frame, constants in `UI/BootTiming.h`, shared with the startup chime.
- Ridge: present from the first frame; it is ground.
- Wind lines: the first starts at 80 ms, each next one 90 ms later, each takes 520 ms to cross the sign, ease-out cubic.
- Wing: appears at 300 ms, 20 px below its resting height, over the crest.
- Results: the first row shows at 900 ms, then one every 300 ms (last at 1800 ms). Rows appear whole; only the wing moves, rising over 250 ms (ease-out cubic) after each passed result.
- Hold: 900 ms after the last result when all passed, 2200 ms when something failed. Then a hard cut to Flight, no transition.
- Any button skips straight to Flight. The art is redrawn only while it moves; the held frame costs nothing.

#### Startup chime
The boot sign has a sound, written as MIDI notes in `UI/BootChime.cpp` and played on the piezo by `MelodyPlayer` (one square-wave note at a time). It follows the same clock and the same pass/fail rule (`BootTiming::checksPassed`) as the rows:
- Wind: a quick rising run, D5 E5 F#5 A5 B5 (40 ms each), as the wing appears at 300 ms.
- Each result: a passed check is the vario's climb double-beep, 70 ms on, 40 off, 70 on, a step higher each time (D6, E6, F#6, A6), so passing sounds like climbing. A failed check is one low A4 "sink" tone, 260 ms.
- Ready, 400 ms after the last result: rising A6 then D7 when all passed; falling F#5 then D5 when anything failed.
- D major pentatonic set high, because a piezo is loudest and cleanest from about 1 to 3 kHz, where a vario climbs.
- Volume OFF in Settings silences it, and the vario's own tones are muted while it plays. A skip cuts it off.

**The Sound Agrees With The Sign Rule.** The chime is composed from the same check results and the same timing as the rows; a failure that shows yellow is heard low. Never let one change without the other.

`bayupandu_sim --export-chime DIR` writes it as `.mid` (General MIDI square lead, 1 tick = 1 ms) and `.wav` (through the simulator's piezo-like synth); the copies in `assets/sound/` come from there.

### Navigation (buttons)
Four hardware buttons, no touch. UP/DOWN cycle the five pages (Flight, Thermal, Navigation, Status, Settings); CANCEL returns to Flight; ENTER enters edit mode on Settings. In edit mode UP/DOWN move focus, ENTER changes the focused value, CANCEL saves and leaves. The error sign appears once when an error starts; UP, DOWN or CANCEL leaves it for Flight.

## Do's and Don'ts

### Do:
- **Do** build every page from one 232x312 sign panel at (4,4) with a 3 px border and 8 px corners.
- **Do** separate the hero from the distance lines with the single 3 px white rule.
- **Do** print hero figures as magnitude only and let the arrow carry direction.
- **Do** show an unknown figure as a single "-" and hide its glyph.
- **Do** use the seven generated fonts at their generated sizes only.
- **Do** keep the LVGL heap budget in mind: 64 KB total, one page set about 48 KB. Free the old pages before rebuilding (language change), never hold two sets.
- **Do** invert the focused settings row (white band, green text) as the only focus treatment.
- **Do** draw the brand mark from its geometry (`BrandMark.cpp` / `gen_brand.py`) in one colour: white on green, ink on light, white on dark.
- **Do** mark a failed boot check with the yellow band and diamond together, and withhold its lift.
- **Do** free the boot sign before building the flight pages.

### Don't:
- **Don't** use warning yellow for anything that is not a warning or the error sign.
- **Don't** show stale data as a figure, struck through or dimmed; a struck-through number still reads as live in a one-second glance.
- **Don't** show the level bar for an unknown value; it would claim level flight.
- **Don't** use cards, nested boxes, shadows, gradients or transparency.
- **Don't** signal anything with colour alone; pair it with a glyph, shape or position.
- **Don't** add a side vario bar or a grid of boxed readouts.
- **Don't** animate anything outside the boot sign; flight pages change by value only.
- **Don't** colour the brand mark yellow or two-tone it, or ship it from a raster source.
- **Don't** scale the vector mark down to 16 px; use the hand-fitted favicon pixel map.

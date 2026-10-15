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
---

# Design System: BayuPandu

## Overview

**Creative North Star: "The Direction Sign"**

Every page is one highway guide sign: a retroreflective green panel with a white border, white legends in a road-sign face, and a single white rule dividing the sign into sections. Figures are set as sign legends, big and heavy, so climb, height and glide read in about one second from a cockpit pod in direct sun. The system refuses the instrument-category default of a grid of boxes with a side vario bar.

Density is low by design. Each page has one hero (a sign glyph beside an 84 px figure) and a short stack of distance lines below it. Meaning is carried by shape and position as well as colour: lift and sink are different arrows, warnings are a different sign. There is no depth, no ornament and no motion beyond values changing on a 200 ms refresh.

The palette has one job per colour. Green is the ground, white is every legend, yellow means something is wrong and nothing else.

**Key Characteristics:**
- One road-sign panel per page, 3 px white border, 8 px corners, on a near-black surround.
- Hero row: polygon sign glyph plus 84 px Overpass Black figure, magnitude only.
- Distance lines: key left, 38 px figure right, unit in its own column.
- Yellow is reserved for warnings and the error sign.
- Unknown values are a single dash with the glyph hidden.
- Flat: no cards, shadows, gradients or colour-only meaning.

## Colors

A road-sign palette: one saturated ground, one legend colour, one warning colour and its ink.

### Primary
- **Retroreflective Sign Green** (`kSignGreen`): the ground of every normal page panel, and the text colour of a focused (inverted) settings row.

### Secondary
- **Warning Sign Yellow** (`kWarning`): only for warnings. Fills the footer when a warning is up and fills the whole panel on the error page. Never used for data, focus or decoration.

### Neutral
- **Legend White** (`kLegend`): every legend, figure, glyph, the panel border, the section rule, and the band of a focused settings row.
- **Warning Ink** (`kInk`): legends, border and the diamond glyph on yellow. Appears only on yellow.
- **Cab Surround** (`kSurround`): the 4 px screen margin around the sign panel, a green-tinted near-black so the panel reads as a mounted sign rather than a full-bleed screen.

### Named Rules
**The Yellow Means Trouble Rule.** Warning yellow appears only when something is wrong: a footer warning sign or the error panel. If a surface is yellow and nothing is wrong, it is a bug.

**The Shape Carries Meaning Rule.** Every colour signal has a shape twin: the footer warning adds the diamond glyph, sink is a down arrow and not a red number, level is a bar. The UI must read correctly on a monochrome panel.

## Typography

**Display Font:** Overpass Black (wght 900), figures only
**Body Font:** Overpass ExtraBold (wght 800)

**Character:** Overpass descends from US Highway Gothic, so the legends are literally road-sign lettering. Black for figures, ExtraBold for words; no lighter weights exist on the device.

Fonts are 4 bpp LVGL bitmaps generated by `firmware/tools/gen_fonts.sh` at fixed sizes with limited glyph ranges. A size or glyph not in that script does not exist on the device.

### Hierarchy
- **Display** (Black, 84 px, `bp_figures_84`, glyphs `-./0-9` only): the hero figure on Flight, Thermal and Navigation.
- **Headline** (Black, 38 px, `bp_figures_38`, digits, sign, `.:%`, `k`, `m`, degree): distance-line figures, and the fallback for a hero figure too wide for its room.
- **Title** (ExtraBold, 22 px, `bp_text_22`): two-line row values on Status and Settings, the Navigation destination legend, the error headline.
- **Label** (ExtraBold, 14 px, `bp_text_14`, 1 px tracking on page titles and keys, upper case): page title, distance-line keys and units, hero units, footer data and warnings, error advice.
- **Caption** (ExtraBold, 11 px, `bp_text_11`): the status strip, footer button hints, the Navigation position key.

### Named Rules
**The Generated Sizes Rule.** Only the five generated fonts exist. A new size or glyph means editing `gen_fonts.sh` and paying for it in flash; never approximate with scaling.

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

### Named Rules
**The One Sign Rule.** A page is exactly one sign panel. Sections are divided by the white rule, never by nested boxes.

**The Clip, Don't Collide Rule.** When two legends share a line, the lower-priority one gets the remaining width and clips. Hero figures too wide for their room drop to 38 px rather than run into the unit.

## Elevation & Depth

None. The system is flat: one opaque panel on a dark surround, solid fills, no shadows, gradients or transparency. Hierarchy comes from size (84 / 38 / 22 / 14 / 11 px), position, and the white rule.

### Named Rules
**The Flat Sign Rule.** A road sign has no depth. No shadows, gradients, glows or cards, on any page.

## Shapes

Rounded rectangle panel (8 px) with a 3 px border in the legend colour (white on green, ink on yellow). Inner bands (footer warning, focused row) use 3 px corners. Glyphs are filled polygons drawn by `SignGlyph`:
- **Arrow:** a highway-sign arrow, head 92% of the glyph span wide and 62% tall, shaft 36% wide. The head is fixed so it always reads as an arrow; the shaft grows with strength (15% to 100% of the remaining reach). Rotates to any bearing.
- **Level bar:** 92% x 30% of the span, for level flight (|climb| below 0.2 m/s).
- **Diamond:** the warning sign, 14x14 in the footer.

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

### Navigation (buttons)
Four hardware buttons, no touch. UP/DOWN cycle the five pages (Flight, Thermal, Navigation, Status, Settings); CANCEL returns to Flight; ENTER enters edit mode on Settings. In edit mode UP/DOWN move focus, ENTER changes the focused value, CANCEL saves and leaves. The error sign appears once when an error starts; UP, DOWN or CANCEL leaves it for Flight.

## Do's and Don'ts

### Do:
- **Do** build every page from one 232x312 sign panel at (4,4) with a 3 px border and 8 px corners.
- **Do** separate the hero from the distance lines with the single 3 px white rule.
- **Do** print hero figures as magnitude only and let the arrow carry direction.
- **Do** show an unknown figure as a single "-" and hide its glyph.
- **Do** use the five generated fonts at their generated sizes only.
- **Do** keep the LVGL heap budget in mind: 64 KB total, one page set about 48 KB. Free the old pages before rebuilding (language change), never hold two sets.
- **Do** invert the focused settings row (white band, green text) as the only focus treatment.

### Don't:
- **Don't** use warning yellow for anything that is not a warning or the error sign.
- **Don't** show stale data as a figure, struck through or dimmed; a struck-through number still reads as live in a one-second glance.
- **Don't** show the level bar for an unknown value; it would claim level flight.
- **Don't** use cards, nested boxes, shadows, gradients or transparency.
- **Don't** signal anything with colour alone; pair it with a glyph, shape or position.
- **Don't** add a side vario bar or a grid of boxed readouts.

#!/bin/sh
# Regenerates the LVGL bitmap fonts in src/UI/fonts/ from Overpass (SIL OFL 1.1),
# a face derived from the US Highway Gothic road-sign lettering.
# Needs: curl, uvx (fonttools), npx (lv_font_conv).
set -eu

cd "$(dirname "$0")/.."
OUT=src/UI/fonts
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

curl -sSfL -o "$WORK/Overpass.ttf" \
  "https://github.com/google/fonts/raw/main/ofl/overpass/Overpass%5Bwght%5D.ttf"
uvx --from fonttools fonttools varLib.instancer "$WORK/Overpass.ttf" wght=900 -q -o "$WORK/Overpass-Black.ttf"
uvx --from fonttools fonttools varLib.instancer "$WORK/Overpass.ttf" wght=800 -q -o "$WORK/Overpass-ExtraBold.ttf"

conv() { # name ttf size range
  npx --yes lv_font_conv@1.5.2 --font "$WORK/$2" --size "$3" --bpp 4 --no-compress \
    --format lvgl --lv-include lvgl.h -r "$4" -o "$OUT/$1.c"
}

mkdir -p "$OUT"
# Figures only for the big readouts; labels get printable ASCII plus degree and middle dot.
conv bp_figures_84 Overpass-Black.ttf 84 "0x2D-0x39"
conv bp_figures_38 Overpass-Black.ttf 38 "0x20,0x25,0x2B-0x3A,0x6B,0x6D,0xB0"
conv bp_text_22 Overpass-ExtraBold.ttf 22 "0x20-0x7E,0xB0,0xB7"
conv bp_text_14 Overpass-ExtraBold.ttf 14 "0x20-0x7E,0xB0,0xB7"
conv bp_text_11 Overpass-ExtraBold.ttf 11 "0x20-0x7E,0xB0,0xB7"
# The BAYUPANDU wordmark on the boot screen: only its seven letters.
conv bp_wordmark_32 Overpass-Black.ttf 32 "0x41,0x42,0x44,0x4E,0x50,0x55,0x59"
# Its version line: V, digits and the dot.
conv bp_version_16 Overpass-Black.ttf 16 "0x2E,0x30-0x39,0x56"
echo "fonts written to $OUT"

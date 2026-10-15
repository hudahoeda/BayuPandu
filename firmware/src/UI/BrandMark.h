#pragma once

#include "lvgl.h"

// The BayuPandu mark: a paraglider wing seen from the front, an arc of cells,
// carried on three short wind lines. Drawn as flat polygons so it stays sharp at
// any size and costs no image flash.
//
// The geometry is defined for a mark 1 unit wide, origin at the top centre of the
// wing. assets/brand/gen_brand.py draws the SVG versions from the same numbers;
// change both together.
namespace BrandMark {

// Height of the whole mark (wing plus wind lines) for a given width.
float heightFor(float width);

// Draws the mark with its top centre at (cx, top), `width` pixels wide.
void draw(lv_draw_ctx_t* ctx, float cx, float top, float width, lv_color_t color);

}  // namespace BrandMark

#pragma once

#include "lvgl.h"

// Road-sign glyphs drawn as filled polygons, so they scale and rotate without
// bitmap icons: the direction arrow, the "level" bar, and the warning diamond.
namespace SignGlyph {

enum class Shape : uint8_t { ARROW, LEVEL, DIAMOND };

// Creates a glyph object of the given box size; draws centred in it.
lv_obj_t* create(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, lv_color_t color);

void setShape(lv_obj_t* glyph, Shape shape);
// Arrow heading in degrees clockwise from straight up (0 = up, 180 = down).
void setAngle(lv_obj_t* glyph, float degrees);
// Arrow length as a fraction of the box's shorter-axis budget, 0..1.
void setLength(lv_obj_t* glyph, float fraction);
void setColor(lv_obj_t* glyph, lv_color_t color);

}  // namespace SignGlyph

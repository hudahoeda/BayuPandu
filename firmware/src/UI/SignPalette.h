#pragma once

#include "lvgl.h"

// Road-sign palette shared by every screen: retroreflective green, white legends,
// warning yellow (only when something is wrong) and the ink used on yellow.
namespace SignPalette {

inline const lv_color_t kSignGreen = lv_color_hex(0x00613F);
inline const lv_color_t kLegend = lv_color_hex(0xFFFFFF);
inline const lv_color_t kWarning = lv_color_hex(0xFFC20E);
inline const lv_color_t kInk = lv_color_hex(0x111111);
inline const lv_color_t kSurround = lv_color_hex(0x0E1A14);

}  // namespace SignPalette

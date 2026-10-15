#include "BrandMark.h"
#include <cmath>
#include <initializer_list>

namespace BrandMark {

namespace {

constexpr float kDegToRad = static_cast<float>(M_PI) / 180.0f;

// Wing: cells between an outer arc and an inner arc that tapers toward the tips.
constexpr float kHalfSpanDeg = 42.0f;     // arc reaches 42 degrees each side of vertical
constexpr int kCells = 9;
constexpr float kGapDeg = 1.8f;           // gap between cells
constexpr float kThickCentre = 0.18f;     // cell depth at the centre
constexpr float kThickTip = 0.11f;        // cell depth at the tips
// Outer radius so the outer arc's chord is 1 unit wide.
const float kRadius = 0.5f / std::sin(kHalfSpanDeg * kDegToRad);

// Wind lines under the wing: y of the line's centre, start and end x.
struct WindLine { float y, x0, x1; };
constexpr WindLine kWind[] = {
    {0.345f, -0.18f, 0.22f},
    {0.410f, -0.17f, 0.21f},
    {0.475f, -0.07f, 0.11f},
};
constexpr float kWindThick = 0.034f;

float thicknessAt(float deg) {
    float f = deg / kHalfSpanDeg;
    return kThickCentre - (kThickCentre - kThickTip) * f * f;
}

// Point on the wing at radius `rho` from the arc centre, `deg` from vertical.
lv_point_t wingPoint(float rho, float deg, float cx, float top, float w) {
    float a = deg * kDegToRad;
    lv_point_t p;
    p.x = static_cast<lv_coord_t>(std::lround(cx + w * rho * std::sin(a)));
    p.y = static_cast<lv_coord_t>(std::lround(top + w * (kRadius - rho * std::cos(a))));
    return p;
}

}  // namespace

float heightFor(float width) {
    return width * (kWind[2].y + kWindThick / 2.0f);
}

void draw(lv_draw_ctx_t* ctx, float cx, float top, float width, lv_color_t color) {
    lv_draw_rect_dsc_t fill;
    lv_draw_rect_dsc_init(&fill);
    fill.bg_color = color;
    fill.bg_opa = LV_OPA_COVER;

    float cellDeg = (2.0f * kHalfSpanDeg - (kCells - 1) * kGapDeg) / kCells;
    for (int i = 0; i < kCells; i++) {
        float a0 = -kHalfSpanDeg + i * (cellDeg + kGapDeg);
        float a1 = a0 + cellDeg;
        // Two points per edge keeps each cell a convex quad, which LVGL fills cleanly.
        lv_point_t quad[4] = {
            wingPoint(kRadius, a0, cx, top, width),
            wingPoint(kRadius, a1, cx, top, width),
            wingPoint(kRadius - thicknessAt(a1), a1, cx, top, width),
            wingPoint(kRadius - thicknessAt(a0), a0, cx, top, width),
        };
        lv_draw_polygon(ctx, &fill, quad, 4);
    }

    // Round the two tips with a disc as deep as the tip cell.
    fill.radius = LV_RADIUS_CIRCLE;
    for (float side : {-1.0f, 1.0f}) {
        lv_point_t c = wingPoint(kRadius - kThickTip / 2.0f, side * kHalfSpanDeg, cx, top, width);
        lv_coord_t r = static_cast<lv_coord_t>(std::lround(width * kThickTip / 2.0f));
        lv_area_t disc = {static_cast<lv_coord_t>(c.x - r), static_cast<lv_coord_t>(c.y - r),
                          static_cast<lv_coord_t>(c.x + r), static_cast<lv_coord_t>(c.y + r)};
        lv_draw_rect(ctx, &fill, &disc);
    }

    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = color;
    line.width = static_cast<lv_coord_t>(std::lround(std::fmax(2.0f, width * kWindThick)));
    line.round_start = 1;
    line.round_end = 1;
    for (const WindLine& wl : kWind) {
        lv_point_t a = {static_cast<lv_coord_t>(std::lround(cx + width * wl.x0)),
                        static_cast<lv_coord_t>(std::lround(top + width * wl.y))};
        lv_point_t b = {static_cast<lv_coord_t>(std::lround(cx + width * wl.x1)), a.y};
        lv_draw_line(ctx, &line, &a, &b);
    }
}

}  // namespace BrandMark

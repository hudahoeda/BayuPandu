#include "SignGlyph.h"
#include <cmath>

namespace SignGlyph {

namespace {

struct GlyphState {
    Shape shape = Shape::ARROW;
    float angle = 0.0f;
    float length = 1.0f;
    lv_color_t color;
};

GlyphState* stateOf(lv_obj_t* obj) {
    return static_cast<GlyphState*>(lv_obj_get_user_data(obj));
}

// Rotates a point given in glyph-local units (origin at centre, -y = forward)
// clockwise by `deg` and moves it to screen coordinates.
lv_point_t place(float x, float y, float deg, float cx, float cy) {
    float r = deg * static_cast<float>(M_PI) / 180.0f;
    float c = std::cos(r), s = std::sin(r);
    lv_point_t p;
    p.x = static_cast<lv_coord_t>(std::lround(cx + x * c - y * s));
    p.y = static_cast<lv_coord_t>(std::lround(cy + x * s + y * c));
    return p;
}

void drawPolygon(lv_draw_ctx_t* ctx, const lv_draw_rect_dsc_t* dsc, const float (*pts)[2], int count,
                 float deg, float cx, float cy) {
    lv_point_t out[8];
    for (int i = 0; i < count; i++) out[i] = place(pts[i][0], pts[i][1], deg, cx, cy);
    lv_draw_polygon(ctx, dsc, out, static_cast<uint16_t>(count));
}

void onDraw(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    GlyphState* st = stateOf(obj);
    if (!st) return;

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    float w = static_cast<float>(lv_area_get_width(&a));
    float h = static_cast<float>(lv_area_get_height(&a));
    float cx = a.x1 + w / 2.0f, cy = a.y1 + h / 2.0f;

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = st->color;
    dsc.bg_opa = LV_OPA_COVER;
    lv_draw_ctx_t* ctx = lv_event_get_draw_ctx(e);

    float span = (w < h ? w : h);
    switch (st->shape) {
        case Shape::ARROW: {
            // Highway-sign arrow: a wide head on a narrower shaft. Head size is fixed so
            // the arrow always reads as an arrow; the shaft grows with strength.
            float headW = span * 0.92f, headH = span * 0.62f, shaftW = span * 0.36f;
            float reach = (w > h ? w : h);
            float total = headH + (reach - headH) * (0.15f + 0.85f * st->length);
            float tip = -total / 2.0f, base = tip + headH, tail = total / 2.0f;
            const float head[3][2] = {{0, tip}, {headW / 2, base}, {-headW / 2, base}};
            const float shaft[4][2] = {{-shaftW / 2, base - 1}, {shaftW / 2, base - 1},
                                       {shaftW / 2, tail}, {-shaftW / 2, tail}};
            drawPolygon(ctx, &dsc, head, 3, st->angle, cx, cy);
            drawPolygon(ctx, &dsc, shaft, 4, st->angle, cx, cy);
            break;
        }
        case Shape::LEVEL: {
            float barW = span * 0.92f, barH = span * 0.30f;
            const float bar[4][2] = {{-barW / 2, -barH / 2}, {barW / 2, -barH / 2},
                                     {barW / 2, barH / 2}, {-barW / 2, barH / 2}};
            drawPolygon(ctx, &dsc, bar, 4, 0.0f, cx, cy);
            break;
        }
        case Shape::DIAMOND: {
            float r = span / 2.0f;
            const float d[4][2] = {{0, -r}, {r, 0}, {0, r}, {-r, 0}};
            drawPolygon(ctx, &dsc, d, 4, 0.0f, cx, cy);
            break;
        }
    }
}

void onDelete(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    delete stateOf(obj);
    lv_obj_set_user_data(obj, nullptr);
}

}  // namespace

lv_obj_t* create(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, lv_color_t color) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, w, h);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    GlyphState* st = new GlyphState();
    st->color = color;
    lv_obj_set_user_data(obj, st);
    lv_obj_add_event_cb(obj, onDraw, LV_EVENT_DRAW_MAIN, nullptr);
    lv_obj_add_event_cb(obj, onDelete, LV_EVENT_DELETE, nullptr);
    return obj;
}

void setShape(lv_obj_t* glyph, Shape shape) {
    GlyphState* st = stateOf(glyph);
    if (st && st->shape != shape) { st->shape = shape; lv_obj_invalidate(glyph); }
}

void setAngle(lv_obj_t* glyph, float degrees) {
    GlyphState* st = stateOf(glyph);
    if (st && std::fabs(st->angle - degrees) > 0.5f) { st->angle = degrees; lv_obj_invalidate(glyph); }
}

void setLength(lv_obj_t* glyph, float fraction) {
    GlyphState* st = stateOf(glyph);
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;
    if (st && std::fabs(st->length - fraction) > 0.01f) { st->length = fraction; lv_obj_invalidate(glyph); }
}

void setColor(lv_obj_t* glyph, lv_color_t color) {
    GlyphState* st = stateOf(glyph);
    if (st) { st->color = color; lv_obj_invalidate(glyph); }
}

}  // namespace SignGlyph

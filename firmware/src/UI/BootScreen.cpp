#include "BootScreen.h"
#include "UI/BrandMark.h"
#include "UI/SignGlyph.h"
#include "UI/SignPalette.h"
#include "version.h"
#include <cmath>
#include <initializer_list>
#include <cstdio>

LV_FONT_DECLARE(bp_wordmark_32);
LV_FONT_DECLARE(bp_version_16);
LV_FONT_DECLARE(bp_text_22);
LV_FONT_DECLARE(bp_text_14);
LV_FONT_DECLARE(bp_text_11);

using namespace SignPalette;

namespace {

// --- Timing (ms from the first frame) -------------------------------------------
constexpr uint32_t kWindStart = 80;       // first wind line starts drawing
constexpr uint32_t kWindDraw = 520;       // time for one line to cross the sign
constexpr uint32_t kWindStagger = 90;     // each line starts this much after the last
constexpr uint32_t kWingAppear = 300;     // wing appears low over the crest as the wind comes in
constexpr uint32_t kFirstReveal = 900;    // first check result shows
constexpr uint32_t kRevealStep = 300;     // then one result every step
constexpr uint32_t kLiftMs = 250;         // the wing's rise after each result
constexpr uint32_t kHoldOk = 900;         // hold after the last result when all passed
constexpr uint32_t kHoldWarn = 2200;      // longer when something failed
constexpr uint32_t kLastReveal = kFirstReveal + (BootScreen::kRows - 1) * kRevealStep;
constexpr uint32_t kAnimEnd = kLastReveal + kLiftMs;

// --- Geometry (screen px). Measured from the approved comp, 1086x1448 scaled to 240x320.
constexpr float kScale = 240.0f / 1086.0f;
constexpr lv_coord_t kArtX = 8, kArtY = 46, kArtW = 224, kArtH = 146;
constexpr float kMarkCx = 181.0f;
constexpr float kMarkTop = 54.0f;
constexpr float kMarkWidth = 84.0f;
// The wing starts kWingRise px low and climbs one equal step per passed check. The
// wind lines are pushed the last kWindPush of their length by the same steps, so
// their arrowheads stay under the wing instead of running into it.
constexpr float kWingRise = 20.0f;
constexpr float kWindPush = 0.15f;
constexpr float kWindWidth = 2.3f;        // stroke widths measured from the comp
constexpr float kRidgeWidth = 2.9f;
constexpr lv_coord_t kRowCapTop[BootScreen::kRows] = {197, 220, 243, 266};
// Columns sit where the comp put them unless a language's longest word needs room;
// then they move left just enough.
constexpr lv_coord_t kKeyX = 51;
constexpr lv_coord_t kValueX = 142;
constexpr lv_coord_t kRowRight = 229;   // last pixel a value may reach
constexpr lv_coord_t kColumnGap = 20;   // never less, so key and value read as two columns
constexpr lv_coord_t kBandX = 10, kBandW = 220;  // warning band, clear of the border

// Stroke centre lines traced from the approved comp, in comp pixels. Drawn as
// Catmull-Rom curves through these points, so the lines stay smooth at any size.
struct Pt { float x, y; };
struct Path { const Pt* pts; int count; };
constexpr Pt kWind1[] = {{52, 651}, {220, 651}, {260, 649}, {300, 645}, {340, 638}, {380, 627},
                         {420, 611}, {460, 589}, {500, 562}, {540, 532}, {580, 503}, {620, 480},
                         {660, 460}, {696, 447}};
constexpr Pt kWind2[] = {{52, 702}, {220, 702}, {260, 701}, {300, 696}, {340, 690}, {380, 679},
                         {420, 665}, {460, 645}, {500, 618}, {540, 586}, {580, 554}, {620, 529},
                         {660, 509}, {700, 496}, {742, 487}};
constexpr Pt kWind3[] = {{52, 750}, {260, 750}, {300, 747}, {340, 740}, {380, 730}, {420, 716},
                         {460, 697}, {500, 671}, {540, 639}, {580, 605}, {620, 577}, {660, 557},
                         {700, 544}, {740, 535}, {796, 529}};
constexpr Pt kRidgePts[] = {{52, 807}, {140, 805}, {180, 802}, {220, 798}, {260, 793}, {300, 787},
                            {340, 780}, {380, 770}, {420, 758}, {460, 742}, {500, 720}, {540, 694},
                            {580, 666}, {620, 638}, {660, 612}, {700, 595}, {735, 589}, {780, 599},
                            {820, 618}, {860, 637}, {900, 648}, {940, 656}, {980, 664}, {1034, 676}};
template <int N> constexpr Path pathOf(const Pt (&pts)[N]) { return {pts, N}; }
constexpr Path kWind[3] = {pathOf(kWind1), pathOf(kWind2), pathOf(kWind3)};
constexpr Path kRidge = pathOf(kRidgePts);
constexpr int kSubSteps = 3;  // stroke segments between two traced points

float easeOut(float x) {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    float inv = 1.0f - x;
    return 1.0f - inv * inv * inv;
}

lv_point_t px(float x, float y) {
    return {static_cast<lv_coord_t>(std::lround(x)), static_cast<lv_coord_t>(std::lround(y))};
}

// Point at parameter u (0..1) between path points i and i+1, on a Catmull-Rom curve,
// in screen pixels.
Pt along(const Path& path, int i, float u) {
    const Pt& p0 = path.pts[i > 0 ? i - 1 : i];
    const Pt& p1 = path.pts[i];
    const Pt& p2 = path.pts[i + 1];
    const Pt& p3 = path.pts[i + 2 < path.count ? i + 2 : i + 1];
    float u2 = u * u, u3 = u2 * u;
    auto cr = [&](float a, float b, float c, float d) {
        return 0.5f * (2 * b + (-a + c) * u + (2 * a - 5 * b + 4 * c - d) * u2 + (-a + 3 * b - 3 * c + d) * u3);
    };
    return {cr(p0.x, p1.x, p2.x, p3.x) * kScale, cr(p0.y, p1.y, p2.y, p3.y) * kScale};
}

// Draws the first `progress` (0..1) of a path as a stroke `width` px wide, and
// reports where it ended and which way it was heading.
//
// LVGL 8.3 puts polygon corners on whole pixels, which leaves 1 px jogs and uneven
// weight on gentle curves. So the stroke is rasterised here: every pixel within
// reach of the centre line takes white in proportion to how much of it the stroke
// covers, from its exact distance to the nearest segment. Only pixels next to the
// line are visited, a few hundred per stroke.
void strokePath(lv_draw_ctx_t* ctx, const Path& path, float progress, float width, lv_color_t color,
                Pt* endPoint = nullptr, Pt* endDir = nullptr) {
    if (progress <= 0.0f) return;
    constexpr int kMaxPts = 24 * kSubSteps + 2;
    Pt pts[kMaxPts];
    int steps = (path.count - 1) * kSubSteps;
    float end = progress * steps;
    int n = 0;
    pts[n++] = along(path, 0, 0.0f);
    for (int k = 1; k <= steps && k - 1 < end && n < kMaxPts; k++) {
        float at = k <= end ? k : end;
        int seg = static_cast<int>(at / kSubSteps);
        if (seg >= path.count - 1) seg = path.count - 2;
        pts[n++] = along(path, seg, at / kSubSteps - seg);
    }
    if (n < 2) return;

    // Squared distance from (x, y) to segment i. Squared keeps the per-pixel work to
    // multiplies; the square root is taken only for pixels that get painted.
    auto distSq = [&](int i, float x, float y) {
        float ax = pts[i].x, ay = pts[i].y, bx = pts[i + 1].x - ax, by = pts[i + 1].y - ay;
        float len2 = bx * bx + by * by;
        float t = len2 > 0 ? ((x - ax) * bx + (y - ay) * by) / len2 : 0.0f;
        t = t < 0 ? 0 : (t > 1 ? 1 : t);
        float dx = x - (ax + bx * t), dy = y - (ay + by * t);
        return dx * dx + dy * dy;
    };

    const lv_area_t* clip = ctx->clip_area;
    const lv_area_t* buf = ctx->buf_area;
    lv_color_t* pixels = static_cast<lv_color_t*>(ctx->buf);
    lv_coord_t bufW = lv_area_get_width(buf);
    float half = width / 2.0f;
    float reach = half + 1.0f;
    float reachSq = (half + 0.5f) * (half + 0.5f);
    for (int i = 0; i + 1 < n; i++) {
        int x0 = static_cast<int>(std::floor(std::fmin(pts[i].x, pts[i + 1].x) - reach));
        int x1 = static_cast<int>(std::ceil(std::fmax(pts[i].x, pts[i + 1].x) + reach));
        int y0 = static_cast<int>(std::floor(std::fmin(pts[i].y, pts[i + 1].y) - reach));
        int y1 = static_cast<int>(std::ceil(std::fmax(pts[i].y, pts[i + 1].y) + reach));
        x0 = LV_MAX(x0, clip->x1); x1 = LV_MIN(x1, clip->x2);
        y0 = LV_MAX(y0, clip->y1); y1 = LV_MIN(y1, clip->y2);
        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x1; x++) {
                float cx = x + 0.5f, cy = y + 0.5f;  // pixel centre
                float d2 = distSq(i, cx, cy);
                if (d2 >= reachSq) continue;
                // A pixel nearer to a neighbouring segment is that segment's to paint.
                if (i > 0 && distSq(i - 1, cx, cy) < d2) continue;
                if (i + 2 < n && distSq(i + 1, cx, cy) <= d2) continue;
                float cover = half + 0.5f - std::sqrt(d2);
                if (cover <= 0.0f) continue;
                lv_opa_t opa = cover >= 1.0f ? LV_OPA_COVER : static_cast<lv_opa_t>(cover * 255.0f);
                lv_color_t& dst = pixels[(y - buf->y1) * bufW + (x - buf->x1)];
                dst = opa == LV_OPA_COVER ? color : lv_color_mix(color, dst, opa);
            }
        }
    }
    auto unit = [](float x, float y) {
        float len = std::sqrt(x * x + y * y);
        return len > 0 ? Pt{x / len, y / len} : Pt{1, 0};
    };
    if (endPoint) *endPoint = pts[n - 1];
    if (endDir) *endDir = unit(pts[n - 1].x - pts[n - 2].x, pts[n - 1].y - pts[n - 2].y);
}

// Sign arrowhead at (x, y) pointing along (dx, dy), at the comp's scale.
void arrowhead(lv_draw_ctx_t* ctx, const lv_draw_rect_dsc_t* dsc, Pt at, Pt dir) {
    const float back = 3.0f, ahead = 7.0f, half = 3.6f;
    float bx = at.x - dir.x * back, by = at.y - dir.y * back;
    lv_point_t tri[3] = {
        px(at.x + dir.x * ahead, at.y + dir.y * ahead),
        px(bx - dir.y * half, by + dir.x * half),
        px(bx + dir.y * half, by - dir.x * half),
    };
    lv_draw_polygon(ctx, dsc, tri, 3);
}

lv_obj_t* plainObj(lv_obj_t* parent) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

// Places a one-line label so its capitals' top edge sits at `capTop`. Overpass
// capitals are 0.70 em tall, and the font's line box has its baseline base_line px
// above the bottom, so the caps start that far below the label's top.
lv_obj_t* legendAtCap(lv_obj_t* parent, const lv_font_t* font, int sizePx, const char* text,
                      lv_coord_t x, lv_coord_t capTop, lv_color_t color) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, text);
    lv_coord_t baseline = font->line_height - font->base_line;
    lv_coord_t capHeight = static_cast<lv_coord_t>(std::lround(0.70f * sizePx));
    lv_obj_set_pos(label, x, capTop - (baseline - capHeight));
    return label;
}

}  // namespace

BootScreen::BootScreen(const UiStrings& t, const SelfTestResult& r, uint32_t nowMs) : startMs(nowMs) {
    root = lv_obj_create(nullptr);
    lv_obj_remove_style_all(root);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root, kSurround, 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    lv_obj_t* panel = plainObj(root);
    lv_obj_set_size(panel, 232, 312);
    lv_obj_set_pos(panel, 4, 4);
    lv_obj_set_style_bg_color(panel, kSignGreen, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, kLegend, 0);
    lv_obj_set_style_border_width(panel, 3, 0);
    lv_obj_set_style_radius(panel, 8, 0);

    art = plainObj(root);
    lv_obj_set_pos(art, kArtX, kArtY);
    lv_obj_set_size(art, kArtW, kArtH);
    lv_obj_set_user_data(art, this);
    lv_obj_add_event_cb(art, onDrawArt, LV_EVENT_DRAW_MAIN, nullptr);

    lv_obj_t* wordmark = legendAtCap(root, &bp_wordmark_32, 32, "BAYUPANDU", 16, 19, kLegend);
    lv_obj_set_style_text_letter_space(wordmark, -1, 0);
    char version[16];
    std::snprintf(version, sizeof(version), "V%s", BAYUPANDU_VERSION);
    legendAtCap(root, &bp_version_16, 16, version, 17, 51, kLegend);

    char gps[24], battery[8];
    if (r.gpsFix && r.satellites > 0) std::snprintf(gps, sizeof(gps), t.satellitesFmt, (unsigned)r.satellites);
    else std::snprintf(gps, sizeof(gps), "%s", r.gpsFix ? t.gpsFix : t.bootNoFix);
    if (!r.gpsResponding) std::snprintf(gps, sizeof(gps), "%s", t.bootGpsSilent);
    std::snprintf(battery, sizeof(battery), "%d%%", r.batteryPercent);
    const char* keys[kRows] = {t.bootBaro, t.bootGps, t.bootCard, t.bootBattery};
    const char* values[kRows] = {r.barometerOk ? t.bootOk : t.bootFault, gps,
                                 r.storageOk ? t.bootOk : t.bootNoCard, battery};
    // No fix yet is normal at switch-on, so only a silent module is a warning.
    const bool warns[kRows] = {!r.barometerOk, !r.gpsResponding, !r.storageOk, r.batteryPercent <= 20};

    // Size the columns against every word that could appear in this language, so the
    // layout doesn't shift between a passing and a failing boot.
    auto width = [](const char* s) {
        lv_point_t size;
        lv_txt_get_size(&size, s, &bp_text_22, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        return size.x;
    };
    lv_coord_t widestValue = 0, widestKey = 0;
    for (const char* v : {t.bootOk, t.bootFault, t.bootNoCard, t.bootNoFix, t.bootGpsSilent, t.gpsFix, "100%",
                          "12 SAT"}) {
        widestValue = LV_MAX(widestValue, width(v));
    }
    for (const char* k : keys) widestKey = LV_MAX(widestKey, width(k));
    valueX = LV_MIN(kValueX, kRowRight - widestValue);
    keyX = LV_MIN(kKeyX, valueX - kColumnGap - widestKey);

    for (int i = 0; i < kRows; i++) buildRow(i, keys[i], values[i], warns[i]);

    lv_obj_t* caption = legendAtCap(root, &bp_text_11, 11, t.bootSkip, 0, 298, kLegend);
    lv_obj_set_style_text_letter_space(caption, 1, 0);
    lv_obj_set_width(caption, 240);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
}

BootScreen::~BootScreen() {
    if (root) lv_obj_del(root);
}

void BootScreen::buildRow(int index, const char* key, const char* value, bool warn) {
    Row& row = rows[index];
    row.warn = warn;
    anyWarning |= warn;
    lv_coord_t capTop = kRowCapTop[index];
    lv_color_t ink = warn ? kInk : kLegend;
    if (warn) {
        row.band = plainObj(root);
        lv_obj_set_pos(row.band, kBandX, capTop - 5);
        lv_obj_set_size(row.band, kBandW, 26);
        lv_obj_set_style_bg_color(row.band, kWarning, 0);
        lv_obj_set_style_bg_opa(row.band, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row.band, 3, 0);
        row.glyph = SignGlyph::create(root, 12, 12, kInk);
        SignGlyph::setShape(row.glyph, SignGlyph::Shape::DIAMOND);
        lv_obj_set_pos(row.glyph, kBandX + 5, capTop + 2);
    }
    row.key = legendAtCap(root, &bp_text_22, 22, key, keyX, capTop, ink);
    row.value = legendAtCap(root, &bp_text_22, 22, value, valueX, capTop, ink);
    for (lv_obj_t* obj : {row.band, row.glyph, row.key, row.value}) {
        if (obj) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

bool BootScreen::update(uint32_t nowMs) {
    if (skipped) return false;
    elapsedMs = nowMs - startMs;

    for (int i = 0; i < kRows; i++) {
        if (elapsedMs < kFirstReveal + i * kRevealStep) break;
        for (lv_obj_t* obj : {rows[i].band, rows[i].glyph, rows[i].key, rows[i].value}) {
            if (obj && lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
    }
    // Redraw the wind and wing only while they move; the held frame costs nothing.
    if (elapsedMs <= kAnimEnd + 40) lv_obj_invalidate(art);

    return elapsedMs < kLastReveal + (anyWarning ? kHoldWarn : kHoldOk);
}

void BootScreen::onDrawArt(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    const BootScreen* self = static_cast<const BootScreen*>(lv_obj_get_user_data(obj));
    if (self) self->drawArt(lv_event_get_draw_ctx(e));
}

void BootScreen::drawArt(lv_draw_ctx_t* ctx) const {
    float t = static_cast<float>(elapsedMs);

    lv_draw_rect_dsc_t fill;
    lv_draw_rect_dsc_init(&fill);
    fill.bg_color = kLegend;
    fill.bg_opa = LV_OPA_COVER;

    // Lift comes only from checks that passed: a failure leaves the wing lower.
    float lifted = 0.0f;
    for (int i = 0; i < kRows; i++) {
        if (rows[i].warn) continue;
        lifted += easeOut((t - (kFirstReveal + i * kRevealStep)) / kLiftMs);
    }
    float lift = lifted / kRows;  // 0 at the start, 1 when every check passed

    // The ridge is ground: it is there from the first frame.
    strokePath(ctx, kRidge, 1.0f, kRidgeWidth, kLegend);

    for (int i = 0; i < 3; i++) {
        float start = kWindStart + i * kWindStagger;
        float drawn = easeOut((t - start) / kWindDraw) * (1.0f - kWindPush) + kWindPush * lift;
        Pt tip, dir;
        if (drawn <= 0.02f) continue;
        strokePath(ctx, kWind[i], drawn, kWindWidth, kLegend, &tip, &dir);
        arrowhead(ctx, &fill, tip, dir);
    }

    if (t >= kWingAppear) {
        BrandMark::draw(ctx, kMarkCx, kMarkTop + kWingRise * (1.0f - lift), kMarkWidth, kLegend);
    }
}

#include "UserInterface.h"
#include "UI/SignGlyph.h"
#include <cmath>
#include <cstdio>
#include <cstring>

LV_FONT_DECLARE(bp_figures_84);
LV_FONT_DECLARE(bp_figures_38);
LV_FONT_DECLARE(bp_text_22);
LV_FONT_DECLARE(bp_text_14);
LV_FONT_DECLARE(bp_text_11);

namespace {

// Road-sign palette: retroreflective green, white legends, warning yellow.
const lv_color_t kSignGreen = lv_color_hex(0x00613F);
const lv_color_t kLegend = lv_color_hex(0xFFFFFF);
const lv_color_t kWarning = lv_color_hex(0xFFC20E);
const lv_color_t kInk = lv_color_hex(0x111111);
const lv_color_t kSurround = lv_color_hex(0x0E1A14);

// Geometry, in panel content coordinates (panel 232x312 at 4,4 with a 3 px border).
constexpr lv_coord_t kContentW = 226;
constexpr lv_coord_t kInset = 6;
constexpr lv_coord_t kInnerW = kContentW - 2 * kInset;
constexpr lv_coord_t kStripH = 26;
constexpr lv_coord_t kFooterY = 280;
constexpr lv_coord_t kFooterH = 22;
constexpr lv_coord_t kUnitColumn = 46;

// Hero row: sign glyph on the left, figure starting a fixed gap after it, and the unit
// tucked under the figure's right end so it never competes for width.
constexpr lv_coord_t kHeroGlyphX = kInset + 2;
constexpr lv_coord_t kHeroGlyphW = 48;
constexpr lv_coord_t kHeroGap = 8;
constexpr lv_coord_t kHeroFigureX = kHeroGlyphX + kHeroGlyphW + kHeroGap;
constexpr lv_coord_t kHeroFigureMaxW = kContentW - kHeroFigureX - kInset;
constexpr lv_coord_t kHeroUnitY = 114;  // just above the rule at 136

// Shown in place of a figure that isn't known. One dash: two read as a glitch at 38 px.
constexpr const char* kNoFigure = "-";

lv_obj_t* plainObj(lv_obj_t* parent) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

lv_obj_t* legend(lv_obj_t* parent, const lv_font_t* font, const char* textValue = "") {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, kLegend, 0);
    lv_label_set_text(label, textValue);
    return label;
}

lv_obj_t* rule(lv_obj_t* parent, lv_coord_t y) {
    lv_obj_t* line = plainObj(parent);
    lv_obj_set_size(line, kInnerW, 3);
    lv_obj_set_pos(line, kInset, y);
    lv_obj_set_style_bg_color(line, kLegend, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    return line;
}

void setText(lv_obj_t* label, const char* value) {
    if (label && std::strcmp(lv_label_get_text(label), value) != 0) lv_label_set_text(label, value);
}

// Hero figures print the magnitude only: the sign arrow beside them carries direction.
void formatClimb(char* out, size_t n, float vs, bool signedText = true) {
    float mag = std::fabs(vs);
    const char* sign = (signedText && vs <= -0.05f) ? "-" : "";
    if (mag >= 9.95f) std::snprintf(out, n, "%s%.0f", sign, mag);
    else std::snprintf(out, n, "%s%.1f", sign, mag);
}

}  // namespace

UserInterface::UserInterface(
    FlightManager& flightManager,
    ConfigService& configService,
    IArduino& arduino,
    InputManager& inputManager
) :
    flightManager(flightManager),
    configService(configService),
    arduino(arduino),
    inputManager(inputManager)
{
}

const UiStrings& UserInterface::text() const {
    return uiStrings(configService.getConfig().language);
}

void UserInterface::initialize() {
    if (!LVGLInit::initialize(arduino)) {
        return;
    }
    buildAll();
    setScreen(DisplayScreen::MAIN_FLIGHT);
    lastUpdate = arduino.millis();
}

// ---------------------------------------------------------------------------
// Building

void UserInterface::buildAll() {
    // Free the old pages before building new ones: two full sets of pages don't fit
    // in the LVGL heap, and an LVGL allocation failure halts the device. A bare
    // placeholder screen stays active meanwhile, since the active screen can't be deleted.
    lv_obj_t* placeholder = nullptr;
    if (pages[0].screen) {
        placeholder = lv_obj_create(nullptr);
        lv_obj_remove_style_all(placeholder);
        lv_scr_load(placeholder);
        for (int i = 0; i <= kPageCount; i++) {
            if (pages[i].screen) lv_obj_del(pages[i].screen);
            pages[i] = SignPage();
        }
    }

    builtLanguage = configService.getConfig().language;
    buildFlightPage();
    buildThermalPage();
    buildNavigationPage();
    buildStatusPage();
    buildSettingsPage();
    buildErrorPage();

    lv_scr_load(pages[static_cast<int>(currentScreen)].screen);
    if (placeholder) lv_obj_del(placeholder);
    refresh();
}

UserInterface::SignPage UserInterface::buildPage(DisplayScreen screen, bool warningGround) {
    SignPage page;
    page.screen = lv_obj_create(nullptr);
    lv_obj_remove_style_all(page.screen);
    lv_obj_clear_flag(page.screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(page.screen, kSurround, 0);
    lv_obj_set_style_bg_opa(page.screen, LV_OPA_COVER, 0);

    lv_color_t ground = warningGround ? kWarning : kSignGreen;
    lv_color_t ink = warningGround ? kInk : kLegend;

    page.panel = plainObj(page.screen);
    lv_obj_set_size(page.panel, 232, 312);
    lv_obj_set_pos(page.panel, 4, 4);
    lv_obj_set_style_bg_color(page.panel, ground, 0);
    lv_obj_set_style_bg_opa(page.panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(page.panel, ink, 0);
    lv_obj_set_style_border_width(page.panel, 3, 0);
    lv_obj_set_style_radius(page.panel, 8, 0);

    const UiStrings& t = text();
    // The error panel's own legend names the problem, so it carries no title.
    const char* titles[] = {t.pageFlight, t.pageThermal, t.pageNav, t.pageStatus, t.pageSettings, ""};
    page.title = legend(page.panel, &bp_text_14, titles[static_cast<int>(screen)]);
    lv_obj_set_style_text_color(page.title, ink, 0);
    lv_obj_set_style_text_letter_space(page.title, 1, 0);
    lv_obj_set_pos(page.title, kInset + 3, 5);

    // The strip gets whatever the title leaves, clipped, so they can never collide.
    lv_point_t titleSize;
    lv_txt_get_size(&titleSize, lv_label_get_text(page.title), &bp_text_14, 1, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    page.strip = legend(page.panel, &bp_text_11);
    lv_obj_set_style_text_color(page.strip, ink, 0);
    lv_obj_set_style_text_align(page.strip, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_long_mode(page.strip, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(page.strip, kInnerW - 6 - titleSize.x - 10);
    lv_obj_align(page.strip, LV_ALIGN_TOP_RIGHT, -kInset - 3, 7);

    if (warningGround) return page;

    page.footer = plainObj(page.panel);
    lv_obj_set_size(page.footer, kInnerW, kFooterH);
    lv_obj_set_pos(page.footer, kInset, kFooterY);
    lv_obj_set_style_radius(page.footer, 3, 0);
    lv_obj_set_style_bg_color(page.footer, kWarning, 0);
    lv_obj_set_style_bg_opa(page.footer, LV_OPA_TRANSP, 0);

    page.footerGlyph = SignGlyph::create(page.footer, 14, 14, kInk);
    SignGlyph::setShape(page.footerGlyph, SignGlyph::Shape::DIAMOND);
    lv_obj_align(page.footerGlyph, LV_ALIGN_LEFT_MID, 6, 0);

    page.footerText = legend(page.footer, &bp_text_14);
    lv_obj_align(page.footerText, LV_ALIGN_CENTER, 0, 1);
    return page;
}

UserInterface::SignRow UserInterface::buildRow(lv_obj_t* panel, lv_coord_t y, lv_coord_t height,
                                               const lv_font_t* valueFont) {
    SignRow row;
    row.band = plainObj(panel);
    lv_obj_set_size(row.band, kInnerW, height);
    lv_obj_set_pos(row.band, kInset, y);
    lv_obj_set_style_radius(row.band, 3, 0);
    lv_obj_set_style_bg_color(row.band, kLegend, 0);
    lv_obj_set_style_bg_opa(row.band, LV_OPA_TRANSP, 0);

    // Figures and labels sit on a shared baseline near the band's bottom.
    bool bigFigures = valueFont == &bp_figures_38;
    lv_coord_t baselineNudge = bigFigures ? 4 : 1;

    row.key = legend(row.band, &bp_text_14);
    lv_obj_set_style_text_letter_space(row.key, 1, 0);
    lv_obj_align(row.key, LV_ALIGN_LEFT_MID, 3, bigFigures ? 6 : 1);

    row.value = legend(row.band, valueFont);
    row.unit = legend(row.band, &bp_text_14);
    if (bigFigures) {
        lv_obj_align(row.value, LV_ALIGN_RIGHT_MID, -kUnitColumn, baselineNudge);
        lv_obj_align(row.unit, LV_ALIGN_RIGHT_MID, -3, 6);
    } else {
        lv_obj_align(row.value, LV_ALIGN_RIGHT_MID, -3, baselineNudge);
        lv_obj_add_flag(row.unit, LV_OBJ_FLAG_HIDDEN);
    }
    return row;
}

void UserInterface::buildFlightPage() {
    SignPage& page = pages[static_cast<int>(DisplayScreen::MAIN_FLIGHT)];
    page = buildPage(DisplayScreen::MAIN_FLIGHT);
    const UiStrings& t = text();

    // Climb: the sign's direction arrow beside the figure.
    climbArrow = SignGlyph::create(page.panel, kHeroGlyphW, 100, kLegend);
    lv_obj_set_pos(climbArrow, kHeroGlyphX, kStripH + 4);

    climbValue = legend(page.panel, &bp_figures_84, "0.0");
    lv_obj_set_pos(climbValue, kHeroFigureX, kStripH + 18);
    climbUnit = legend(page.panel, &bp_text_14, "m/s");
    lv_obj_align(climbUnit, LV_ALIGN_TOP_RIGHT, -kInset - 3, kHeroUnitY);

    rule(page.panel, 136);

    altRow = buildRow(page.panel, 142, 44, &bp_figures_38);
    speedRow = buildRow(page.panel, 186, 44, &bp_figures_38);
    glideRow = buildRow(page.panel, 230, 44, &bp_figures_38);
    lv_label_set_text(altRow.key, t.alt);
    lv_label_set_text(altRow.unit, "m");
    lv_label_set_text(speedRow.key, t.groundSpeed);
    lv_label_set_text(speedRow.unit, "km/h");
    lv_label_set_text(glideRow.key, t.glide);
    lv_label_set_text(glideRow.unit, "");
}

void UserInterface::buildThermalPage() {
    SignPage& page = pages[static_cast<int>(DisplayScreen::THERMAL)];
    page = buildPage(DisplayScreen::THERMAL);
    const UiStrings& t = text();

    // Same hero as Flight, but for the thermal's average: is this one worth staying in?
    thermalArrow = SignGlyph::create(page.panel, kHeroGlyphW, 100, kLegend);
    lv_obj_set_pos(thermalArrow, kHeroGlyphX, kStripH + 4);
    thermalCaption = legend(page.panel, &bp_text_14, t.thermalNone);
    lv_obj_set_style_text_letter_space(thermalCaption, 1, 0);
    lv_obj_align(thermalCaption, LV_ALIGN_TOP_RIGHT, -kInset - 3, kStripH + 2);
    // Figure sits lower than on Flight to clear the caption above it.
    thermalValue = legend(page.panel, &bp_figures_84, kNoFigure);
    lv_obj_set_pos(thermalValue, kHeroFigureX, kStripH + 28);
    lv_obj_t* unit = legend(page.panel, &bp_text_14, "m/s");
    lv_obj_align(unit, LV_ALIGN_TOP_RIGHT, -kInset - 3, kHeroUnitY);

    rule(page.panel, 136);

    gainRow = buildRow(page.panel, 142, 44, &bp_figures_38);
    thermalTimeRow = buildRow(page.panel, 186, 44, &bp_figures_38);
    thermalAltRow = buildRow(page.panel, 230, 44, &bp_figures_38);
    lv_label_set_text(gainRow.key, t.gain);
    lv_label_set_text(gainRow.unit, "m");
    lv_label_set_text(thermalTimeRow.key, t.timeInThermal);
    lv_label_set_text(thermalTimeRow.unit, "");
    lv_label_set_text(thermalAltRow.key, t.alt);
    lv_label_set_text(thermalAltRow.unit, "m");
}

void UserInterface::buildNavigationPage() {
    SignPage& page = pages[static_cast<int>(DisplayScreen::NAVIGATION)];
    page = buildPage(DisplayScreen::NAVIGATION);
    const UiStrings& t = text();

    // Way back to takeoff: arrow relative to the current track, distance as the legend.
    // Square box so the arrow can turn to any bearing without clipping.
    takeoffArrow = SignGlyph::create(page.panel, kHeroGlyphW, kHeroGlyphW, kLegend);
    lv_obj_set_pos(takeoffArrow, kHeroGlyphX, kStripH + 38);

    // Guide-sign order: destination legend, arrow, distance.
    takeoffKey = legend(page.panel, &bp_text_22, t.takeoff);
    lv_obj_align(takeoffKey, LV_ALIGN_TOP_RIGHT, -kInset - 3, kStripH);
    takeoffValue = legend(page.panel, &bp_figures_84, kNoFigure);
    lv_obj_set_pos(takeoffValue, kHeroFigureX, kStripH + 32);
    takeoffUnit = legend(page.panel, &bp_text_14, "km");
    lv_obj_align(takeoffUnit, LV_ALIGN_TOP_RIGHT, -kInset - 3, kHeroUnitY);

    rule(page.panel, 136);

    trackRow = buildRow(page.panel, 142, 44, &bp_figures_38);
    gpsAltRow = buildRow(page.panel, 186, 44, &bp_figures_38);
    lv_label_set_text(trackRow.key, t.track);
    lv_label_set_text(trackRow.unit, "");
    lv_label_set_text(gpsAltRow.key, t.gpsAlt);
    lv_label_set_text(gpsAltRow.unit, "m");

    positionKey = legend(page.panel, &bp_text_11, t.position);
    lv_obj_set_style_text_letter_space(positionKey, 1, 0);
    lv_obj_set_pos(positionKey, kInset + 3, 236);
    positionValue = legend(page.panel, &bp_text_14, kNoFigure);
    lv_obj_set_pos(positionValue, kInset + 3, 252);
}

void UserInterface::buildStatusPage() {
    SignPage& page = pages[static_cast<int>(DisplayScreen::STATUS)];
    page = buildPage(DisplayScreen::STATUS);
    const UiStrings& t = text();

    rule(page.panel, kStripH + 2);
    SignRow* rows[] = {&systemRow, &gpsRow, &batteryRow, &logRow, &replayRow};
    const char* keys[] = {t.system, t.gps, t.battery, t.flightLog, t.replay};
    for (int i = 0; i < 5; i++) {
        *rows[i] = buildRow(page.panel, kStripH + 10 + i * 48, 44, &bp_text_22);
        lv_label_set_text(rows[i]->key, keys[i]);
        // Two-line rows: key above, value below, so long values never collide.
        lv_obj_align(rows[i]->key, LV_ALIGN_TOP_LEFT, 3, 0);
        lv_obj_align(rows[i]->value, LV_ALIGN_BOTTOM_LEFT, 3, 3);
    }
}

void UserInterface::buildSettingsPage() {
    SignPage& page = pages[static_cast<int>(DisplayScreen::SETTINGS)];
    page = buildPage(DisplayScreen::SETTINGS);
    const UiStrings& t = text();

    rule(page.panel, kStripH + 2);
    const char* keys[] = {t.language, t.volume, t.liftTone};
    for (int i = 0; i < kSettingCount; i++) {
        settingRows[i] = buildRow(page.panel, kStripH + 10 + i * 56, 50, &bp_text_22);
        lv_label_set_text(settingRows[i].key, keys[i]);
        lv_obj_align(settingRows[i].key, LV_ALIGN_TOP_LEFT, 5, 3);
        lv_obj_align(settingRows[i].value, LV_ALIGN_BOTTOM_LEFT, 5, 1);
    }
}

void UserInterface::buildErrorPage() {
    SignPage& page = pages[static_cast<int>(DisplayScreen::ERROR)];
    page = buildPage(DisplayScreen::ERROR, true);
    const UiStrings& t = text();

    // The whole panel is the warning sign; the legend says what happened.
    errorMessage = lv_label_create(page.panel);
    lv_obj_set_style_text_font(errorMessage, &bp_text_22, 0);
    lv_obj_set_style_text_color(errorMessage, kInk, 0);
    lv_obj_set_style_text_align(errorMessage, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(errorMessage, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(errorMessage, kInnerW);
    lv_obj_align(errorMessage, LV_ALIGN_TOP_MID, 0, 96);
    lv_label_set_text(errorMessage, t.errorTitle);

    // Firmware detail, as reported by the health monitor (not translated).
    errorDetail = lv_label_create(page.panel);
    lv_obj_set_style_text_font(errorDetail, &bp_text_14, 0);
    lv_obj_set_style_text_color(errorDetail, kInk, 0);
    lv_obj_set_style_text_align(errorDetail, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(errorDetail, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(errorDetail, kInnerW);
    lv_obj_align_to(errorDetail, errorMessage, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
    lv_label_set_text(errorDetail, "");

    // What the pilot should do, in their language.
    errorAdvice = lv_label_create(page.panel);
    lv_obj_set_style_text_font(errorAdvice, &bp_text_14, 0);
    lv_obj_set_style_text_color(errorAdvice, kInk, 0);
    lv_obj_set_style_text_align(errorAdvice, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(errorAdvice, 4, 0);
    lv_obj_set_width(errorAdvice, kInnerW);
    lv_label_set_text(errorAdvice, t.errorAdvice);
    lv_obj_align(errorAdvice, LV_ALIGN_BOTTOM_MID, 0, -48);

    lv_obj_t* hint = lv_label_create(page.panel);
    lv_obj_set_style_text_font(hint, &bp_text_14, 0);
    lv_obj_set_style_text_color(hint, kInk, 0);
    lv_label_set_text(hint, t.errorHint);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -12);
}

// ---------------------------------------------------------------------------
// Updating

void UserInterface::update() {
    if (alertMessage && static_cast<int32_t>(arduino.millis() - alertUntil) >= 0) {
        alertMessage = nullptr;
    }
    if (configService.getConfig().language != builtLanguage) {
        buildAll();
    }
    // Jump to the error sign once, when the error starts; CANCEL can still leave it.
    SystemState state = flightManager.getSystemState();
    if (state == SystemState::ERROR && lastSystemState != SystemState::ERROR) {
        setScreen(DisplayScreen::ERROR);
    }
    lastSystemState = state;

    LVGLInit::handler();

    uint32_t now = arduino.millis();
    if (now - lastUpdate >= kRefreshMs) {
        lastUpdate = now;
        refresh();
    }
}

void UserInterface::refresh() {
    FlightData data = flightManager.getFusedFlightData();
    const FlightStatsSnapshot& stats = flightManager.getFlightStats();
    SignPage& page = pages[static_cast<int>(currentScreen)];
    const UiStrings& t = text();

    if (currentScreen != DisplayScreen::ERROR) {
        updateStrip(page, data, stats);
    }
    const char* warning = currentWarning(data);

    switch (currentScreen) {
        case DisplayScreen::MAIN_FLIGHT: {
            updateFlightPage(data, stats);
            static char avg[32];
            char figure[12];
            formatClimb(figure, sizeof(figure), stats.averageClimb);
            std::snprintf(avg, sizeof(avg), "%s  %s m/s", t.avgClimb, figure);
            updateFooter(page, warning, avg, true);
            break;
        }
        case DisplayScreen::THERMAL:
            updateThermalPage(data, stats);
            updateFooter(page, warning, t.hintPages);
            break;
        case DisplayScreen::NAVIGATION:
            updateNavigationPage(data, stats);
            updateFooter(page, warning, t.hintPages);
            break;
        case DisplayScreen::STATUS:
            updateStatusPage(data);
            updateFooter(page, warning, t.hintPages);
            break;
        case DisplayScreen::SETTINGS:
            updateSettingsPage();
            // While editing, the instructions matter more than a warning the pilot already saw.
            updateFooter(page, editingSettings ? nullptr : warning,
                         editingSettings ? t.hintEditing : t.hintEdit);
            break;
        case DisplayScreen::ERROR: {
            const char* err = flightManager.getLastError();
            setText(errorDetail, err ? err : "");
            break;
        }
    }
}

const char* UserInterface::currentWarning(const FlightData& data) const {
    const UiStrings& t = text();
    BatteryInfo battery = flightManager.getPowerService().getBatteryInfo();
    if (battery.percentage <= 5) return t.warnCriticalBattery;
    if (!data.isValid) return t.warnNoData;
    if (!data.gpsData.hasValidFix) return t.warnNoGps;
    if (battery.percentage <= 20) return t.warnLowBattery;
    return alertMessage;
}

void UserInterface::updateStrip(SignPage& page, const FlightData& data, const FlightStatsSnapshot& stats) {
    const UiStrings& t = text();
    char buf[48];
    int n = 0;
    if (flightManager.isSimulationActive()) n += std::snprintf(buf + n, sizeof(buf) - n, "SIM  ");
    if (!data.gpsData.hasValidFix) {
        n += std::snprintf(buf + n, sizeof(buf) - n, "GPS -  ");
    } else if (data.gpsData.satellites > 0) {
        n += std::snprintf(buf + n, sizeof(buf) - n, t.satellitesFmt, (unsigned)data.gpsData.satellites);
        n += std::snprintf(buf + n, sizeof(buf) - n, "  ");
    } else {
        n += std::snprintf(buf + n, sizeof(buf) - n, "GPS  ");
    }
    if (stats.airborne) {
        uint32_t minutes = stats.airborneSeconds / 60;
        n += std::snprintf(buf + n, sizeof(buf) - n, "%u:%02u  ", (unsigned)(minutes / 60), (unsigned)(minutes % 60));
    }
    std::snprintf(buf + n, sizeof(buf) - n, "%d%%", flightManager.getPowerService().getBatteryInfo().percentage);
    setText(page.strip, buf);
}

void UserInterface::updateFooter(SignPage& page, const char* warning, const char* hint, bool hintIsData) {
    if (!page.footer) return;
    bool warn = warning != nullptr;
    lv_obj_set_style_bg_opa(page.footer, warn ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(page.footerText, warn ? kInk : kLegend, 0);
    if (warn) lv_obj_clear_flag(page.footerGlyph, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(page.footerGlyph, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(page.footerText, (warn || hintIsData) ? &bp_text_14 : &bp_text_11, 0);
    lv_obj_align(page.footerText, LV_ALIGN_CENTER, warn ? 9 : 0, 1);
    setText(page.footerText, warn ? warning : hint);
}

void UserInterface::setClimbGlyph(lv_obj_t* glyph, float vs, bool known) {
    // Up for lift, down for sink, a flat bar when level. Nothing at all when unknown:
    // the bar would claim level flight.
    if (!known) {
        lv_obj_add_flag(glyph, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_clear_flag(glyph, LV_OBJ_FLAG_HIDDEN);
    if (std::fabs(vs) < 0.2f) {
        SignGlyph::setShape(glyph, SignGlyph::Shape::LEVEL);
    } else {
        SignGlyph::setShape(glyph, SignGlyph::Shape::ARROW);
        SignGlyph::setAngle(glyph, vs > 0 ? 0.0f : 180.0f);
        SignGlyph::setLength(glyph, std::fabs(vs) / 4.0f);
    }
}

void UserInterface::setHeroFigure(lv_obj_t* label, const char* value) {
    // Big figures start a fixed gap after the glyph; one too wide for the room left
    // (a three-digit distance) drops to the row size rather than running into the unit.
    lv_point_t size;
    lv_txt_get_size(&size, value, &bp_figures_84, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    const lv_font_t* font = size.x <= kHeroFigureMaxW ? &bp_figures_84 : &bp_figures_38;
    if (lv_obj_get_style_text_font(label, 0) != font) lv_obj_set_style_text_font(label, font, 0);
    setText(label, value);
}

void UserInterface::updateFlightPage(const FlightData& data, const FlightStatsSnapshot& stats) {
    char buf[16];
    // Stale data is never shown as a figure: a struck-through number still reads as live
    // in a one-second glance.
    bool known = data.isValid;

    setClimbGlyph(climbArrow, data.verticalSpeed, known);
    formatClimb(buf, sizeof(buf), data.verticalSpeed, false);
    setHeroFigure(climbValue, known ? buf : kNoFigure);

    std::snprintf(buf, sizeof(buf), "%.0f", data.altitude);
    setText(altRow.value, known ? buf : kNoFigure);

    std::snprintf(buf, sizeof(buf), "%.0f", data.gpsData.speed * 3.6f);
    setText(speedRow.value, (known && data.gpsData.hasValidFix) ? buf : kNoFigure);

    if (known && stats.hasGlideRatio && stats.glideRatio < 99.5f) {
        std::snprintf(buf, sizeof(buf), stats.glideRatio < 19.95f ? "%.1f" : "%.0f", stats.glideRatio);
        setText(glideRow.value, buf);
    } else {
        setText(glideRow.value, kNoFigure);
    }
}

void UserInterface::updateThermalPage(const FlightData& data, const FlightStatsSnapshot& stats) {
    const UiStrings& t = text();
    char buf[16];
    bool known = stats.hasThermal;

    setText(thermalCaption, !known ? t.thermalNone : (stats.inThermal ? t.thermalNow : t.thermalLast));
    setClimbGlyph(thermalArrow, stats.thermalAverage, known);
    formatClimb(buf, sizeof(buf), stats.thermalAverage, false);
    setHeroFigure(thermalValue, known ? buf : kNoFigure);

    std::snprintf(buf, sizeof(buf), "%+.0f", stats.thermalGain);
    setText(gainRow.value, known ? buf : kNoFigure);
    std::snprintf(buf, sizeof(buf), "%u:%02u", (unsigned)(stats.thermalSeconds / 60),
                  (unsigned)(stats.thermalSeconds % 60));
    setText(thermalTimeRow.value, known ? buf : kNoFigure);
    std::snprintf(buf, sizeof(buf), "%.0f", data.altitude);
    setText(thermalAltRow.value, data.isValid ? buf : kNoFigure);
}

void UserInterface::updateNavigationPage(const FlightData& data, const FlightStatsSnapshot& stats) {
    const GPSData& gps = data.gpsData;
    char buf[40];

    if (stats.hasTakeoff && gps.hasValidFix) {
        float km = stats.takeoffDistanceM / 1000.0f;
        // Within 100 m of takeoff there is no meaningful direction to point.
        if (km < 0.1f) {
            lv_obj_add_flag(takeoffArrow, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(takeoffArrow, LV_OBJ_FLAG_HIDDEN);
            SignGlyph::setShape(takeoffArrow, SignGlyph::Shape::ARROW);
            SignGlyph::setLength(takeoffArrow, 1.0f);
            // Pointing relative to where the pilot is flying: straight up means "ahead".
            float relative = std::fmod(stats.takeoffBearingDeg - gps.heading + 360.0f, 360.0f);
            SignGlyph::setAngle(takeoffArrow, relative);
        }
        std::snprintf(buf, sizeof(buf), km < 9.95f ? "%.1f" : "%.0f", km);
        setHeroFigure(takeoffValue, buf);
    } else {
        lv_obj_add_flag(takeoffArrow, LV_OBJ_FLAG_HIDDEN);
        setHeroFigure(takeoffValue, kNoFigure);
    }

    if (gps.hasValidFix) {
        std::snprintf(buf, sizeof(buf), "%03.0f\xC2\xB0", std::fmod(gps.heading + 360.0f, 360.0f));
        setText(trackRow.value, buf);
        std::snprintf(buf, sizeof(buf), "%.0f", gps.altitude);
        setText(gpsAltRow.value, buf);

        double lat = std::fabs(gps.latitude), lon = std::fabs(gps.longitude);
        std::snprintf(buf, sizeof(buf), "%c %02d\xC2\xB0%06.3f'   %c %03d\xC2\xB0%06.3f'",
                      gps.latitude < 0 ? 'S' : 'N', (int)lat, (lat - (int)lat) * 60.0,
                      gps.longitude < 0 ? 'W' : 'E', (int)lon, (lon - (int)lon) * 60.0);
        setText(positionValue, buf);
    } else {
        setText(trackRow.value, kNoFigure);
        setText(gpsAltRow.value, kNoFigure);
        setText(positionValue, kNoFigure);
    }
}

void UserInterface::updateStatusPage(const FlightData& data) {
    const UiStrings& t = text();
    char buf[40];

    const char* state = t.stateStarting;
    switch (flightManager.getSystemState()) {
        case SystemState::INITIALIZING: state = t.stateStarting; break;
        case SystemState::READY: state = t.stateReady; break;
        case SystemState::FLIGHT_ACTIVE: state = t.stateFlying; break;
        case SystemState::LOW_POWER: state = t.stateLowPower; break;
        case SystemState::ERROR: state = t.stateError; break;
    }
    setText(systemRow.value, state);

    if (data.gpsData.hasValidFix && data.gpsData.satellites > 0) {
        int n = std::snprintf(buf, sizeof(buf), "%s  ", t.gpsFix);
        std::snprintf(buf + n, sizeof(buf) - n, t.satellitesFmt, (unsigned)data.gpsData.satellites);
        setText(gpsRow.value, buf);
    } else {
        setText(gpsRow.value, data.gpsData.hasValidFix ? t.gpsFix : t.gpsNoFix);
    }

    BatteryInfo battery = flightManager.getPowerService().getBatteryInfo();
    std::snprintf(buf, sizeof(buf), "%d%%   %.2f V", battery.percentage, battery.voltage);
    setText(batteryRow.value, buf);

    setText(logRow.value, t.logNotBuilt);
    setText(replayRow.value, flightManager.isSimulationActive() ? t.on : t.off);
}

void UserInterface::updateSettingsPage() {
    const UiStrings& t = text();
    const SystemConfig& config = configService.getConfig();
    char buf[24];

    setText(settingRows[0].value, t.languageValue);
    int level = static_cast<int>(std::lround(config.audioVolume / (100.0f / 3.0f)));
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    setText(settingRows[1].value, t.volumeLevels[level]);
    std::snprintf(buf, sizeof(buf), "+%.1f m/s", config.liftThreshold);
    setText(settingRows[2].value, buf);

    // The focused row inverts like a selected sign legend: white band, green figures.
    for (int i = 0; i < kSettingCount; i++) {
        bool focused = editingSettings && i == focusedSetting;
        lv_obj_set_style_bg_opa(settingRows[i].band, focused ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_color_t fg = focused ? kSignGreen : kLegend;
        lv_obj_set_style_text_color(settingRows[i].key, fg, 0);
        lv_obj_set_style_text_color(settingRows[i].value, fg, 0);
    }
}

// ---------------------------------------------------------------------------
// Screens and input

void UserInterface::setScreen(DisplayScreen screen) {
    if (screen != DisplayScreen::SETTINGS && editingSettings) {
        editingSettings = false;
        configService.saveConfig();
    }
    currentScreen = screen;
    lv_obj_t* target = pages[static_cast<int>(screen)].screen;
    if (target && lv_scr_act() != target) lv_scr_load(target);
    refresh();
}

DisplayScreen UserInterface::getCurrentScreen() const {
    return currentScreen;
}

void UserInterface::handleButtonAction(ButtonAction action, uint8_t buttonId) {
    if (action != ButtonAction::SHORT_PRESS) return;
    enum { UP = 0, DOWN = 1, ENTER = 2, CANCEL = 3 };

    if (editingSettings) {
        switch (buttonId) {
            case UP: focusedSetting = (focusedSetting + kSettingCount - 1) % kSettingCount; break;
            case DOWN: focusedSetting = (focusedSetting + 1) % kSettingCount; break;
            case ENTER: changeSetting(focusedSetting); break;
            case CANCEL:
                editingSettings = false;
                configService.saveConfig();
                break;
        }
        refresh();
        return;
    }

    int index = static_cast<int>(currentScreen);
    switch (buttonId) {
        case UP:
            setScreen(static_cast<DisplayScreen>(currentScreen == DisplayScreen::ERROR
                                                     ? 0 : (index + kPageCount - 1) % kPageCount));
            break;
        case DOWN:
            setScreen(static_cast<DisplayScreen>(currentScreen == DisplayScreen::ERROR
                                                     ? 0 : (index + 1) % kPageCount));
            break;
        case ENTER:
            if (currentScreen == DisplayScreen::SETTINGS) {
                editingSettings = true;
                focusedSetting = 0;
                refresh();
            }
            break;
        case CANCEL:
            setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
    }
}

void UserInterface::changeSetting(int index) {
    SystemConfig& config = configService.getConfig();
    switch (index) {
        case 0:
            config.language = config.language == 0 ? 1 : 0;
            buildAll();  // every label changes; rebuild in the new language
            break;
        case 1: {
            int level = static_cast<int>(std::lround(config.audioVolume / (100.0f / 3.0f)));
            level = (level + 1) % 4;
            config.audioVolume = level * (100.0f / 3.0f);
            break;
        }
        case 2: {
            static const float steps[] = {0.1f, 0.2f, 0.3f, 0.5f};
            int next = 0;
            for (int i = 0; i < 4; i++) {
                if (config.liftThreshold < steps[i] + 0.01f) { next = (i + 1) % 4; break; }
            }
            config.liftThreshold = steps[next];
            break;
        }
    }
}

void UserInterface::showAlert(const char* message, uint16_t durationMs) {
    if (message && *message) {
        alertMessage = message;
        alertUntil = arduino.millis() + durationMs;
    }
}

void UserInterface::clearAlert() {
    alertMessage = nullptr;
}

#pragma once

#include "Data/Types.h"
#include "Services/FlightManager.h"
#include "Services/ConfigService.h"
#include "HAL/IArduino.h"
#include "UI/InputManager.h"
#include "UI/LVGLInit.h"
#include "UI/UiStrings.h"
#include "lvgl.h"

enum class DisplayScreen {
    MAIN_FLIGHT,
    THERMAL,
    NAVIGATION,
    STATUS,
    SETTINGS,
    ERROR
};

// The on-device UI: every page is a road-sign panel (green ground, white legends,
// yellow warning signs). UP/DOWN cycle pages, CANCEL returns to Flight, ENTER edits
// on the Settings page.
class UserInterface {
public:
    UserInterface(
        FlightManager& flightManager,
        ConfigService& configService,
        IArduino& arduino,
        InputManager& inputManager
    );

    void initialize();
    void update();

    // Screen management
    void setScreen(DisplayScreen screen);
    DisplayScreen getCurrentScreen() const;

    // Button handling: buttonId 0 = UP, 1 = DOWN, 2 = ENTER, 3 = CANCEL.
    void handleButtonAction(ButtonAction action, uint8_t buttonId);

    // A transient message from the system, shown as a yellow sign in the footer.
    void showAlert(const char* message, uint16_t durationMs = 3000);
    void clearAlert();

private:
    static constexpr int kPageCount = 5;          // pages UP/DOWN cycle through
    static constexpr int kSettingCount = 3;
    static constexpr uint32_t kRefreshMs = 200;

    // One labelled line on a sign: key on the left, figure and unit on the right.
    struct SignRow {
        lv_obj_t* key = nullptr;
        lv_obj_t* value = nullptr;
        lv_obj_t* unit = nullptr;
        lv_obj_t* band = nullptr;   // background for the focused (inverted) state
    };

    struct SignPage {
        lv_obj_t* screen = nullptr;
        lv_obj_t* panel = nullptr;
        lv_obj_t* title = nullptr;
        lv_obj_t* strip = nullptr;
        lv_obj_t* footer = nullptr;      // yellow when a warning is up
        lv_obj_t* footerText = nullptr;
        lv_obj_t* footerGlyph = nullptr; // warning diamond
    };

    FlightManager& flightManager;
    ConfigService& configService;
    IArduino& arduino;
    InputManager& inputManager;

    SignPage pages[kPageCount + 1];   // indexed by DisplayScreen

    // Flight page
    lv_obj_t* climbArrow = nullptr;
    lv_obj_t* climbValue = nullptr;
    lv_obj_t* climbUnit = nullptr;
    SignRow altRow, speedRow, glideRow;

    // Thermal page
    lv_obj_t* thermalArrow = nullptr;
    lv_obj_t* thermalValue = nullptr;
    lv_obj_t* thermalCaption = nullptr;
    SignRow gainRow, thermalTimeRow, thermalAltRow;

    // Navigation page
    lv_obj_t* takeoffArrow = nullptr;
    lv_obj_t* takeoffValue = nullptr;
    lv_obj_t* takeoffUnit = nullptr;
    lv_obj_t* takeoffKey = nullptr;
    SignRow trackRow, gpsAltRow;
    lv_obj_t* positionKey = nullptr;
    lv_obj_t* positionValue = nullptr;

    // Status page
    SignRow systemRow, gpsRow, batteryRow, logRow, replayRow;

    // Settings page
    SignRow settingRows[kSettingCount];
    bool editingSettings = false;
    int focusedSetting = 0;

    // Error page
    lv_obj_t* errorMessage = nullptr;
    lv_obj_t* errorDetail = nullptr;
    lv_obj_t* errorAdvice = nullptr;

    DisplayScreen currentScreen = DisplayScreen::MAIN_FLIGHT;
    SystemState lastSystemState = SystemState::INITIALIZING;
    uint32_t lastUpdate = 0;
    uint8_t builtLanguage = 0xFF;

    const char* alertMessage = nullptr;
    uint32_t alertUntil = 0;

    const UiStrings& text() const;

    // Building
    void buildAll();
    SignPage buildPage(DisplayScreen screen, bool warningGround = false);
    SignRow buildRow(lv_obj_t* panel, lv_coord_t y, lv_coord_t height, const lv_font_t* valueFont);
    void buildFlightPage();
    void buildThermalPage();
    void buildNavigationPage();
    void buildStatusPage();
    void buildSettingsPage();
    void buildErrorPage();

    // Updating
    void refresh();
    void updateStrip(SignPage& page, const FlightData& data, const FlightStatsSnapshot& stats);
    void updateFooter(SignPage& page, const char* warning, const char* hint, bool hintIsData = false);
    const char* currentWarning(const FlightData& data) const;
    void updateFlightPage(const FlightData& data, const FlightStatsSnapshot& stats);
    void updateThermalPage(const FlightData& data, const FlightStatsSnapshot& stats);
    void updateNavigationPage(const FlightData& data, const FlightStatsSnapshot& stats);
    void updateStatusPage(const FlightData& data);
    void updateSettingsPage();
    void setClimbGlyph(lv_obj_t* glyph, float vs, bool known);
    void setHeroFigure(lv_obj_t* label, const char* value);

    // Settings
    void changeSetting(int index);
};

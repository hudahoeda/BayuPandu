#include "UserInterface.h"
#include <string.h>
#include <cstdio>

UserInterface::UserInterface(
    FlightManager& flightManager,
    ConfigService& configService,
    IArduino& arduino
) :
    flightManager(flightManager),
    configService(configService),
    arduino(arduino),
    alertModal(nullptr),
    alertTimer(nullptr),
    currentScreenType(DisplayScreen::MAIN_FLIGHT),
    currentScreenObj(nullptr),
    lastUpdate(0),
    displayActive(true)
{
}

void UserInterface::initialize() {
    // Initialize LVGL if not already done
    if (!LVGLInit::initialize(arduino)) {
        return;
    }
    
    // Create all screens
    mainFlightScreen = createMainFlightScreen();
    navigationScreen = createNavigationScreen();
    settingsScreen = createSettingsScreen();
    statusScreen = createStatusScreen();
    errorScreen = createErrorScreen();
    
    // Set initial screen
    setScreen(DisplayScreen::MAIN_FLIGHT);
    
    lastUpdate = arduino.millis();
    displayActive = true;
}

void UserInterface::update() {
    uint32_t currentTime = arduino.millis();
    
    // Handle LVGL tasks (this will be called from main loop)
    LVGLInit::handler();
    
    // Update current screen data if enough time has passed
    if (shouldUpdateScreen()) {
        switch (currentScreenType) {
            case DisplayScreen::MAIN_FLIGHT:
                updateMainFlightScreen();
                break;
            case DisplayScreen::NAVIGATION:
                updateNavigationScreen();
                break;
            case DisplayScreen::SETTINGS:
                updateSettingsScreen();
                break;
            case DisplayScreen::STATUS:
                updateStatusScreen();
                break;
            case DisplayScreen::ERROR:
                updateErrorScreen();
                break;
        }
        lastUpdate = currentTime;
    }
}

void UserInterface::setScreen(DisplayScreen screen) {
    if (screen == currentScreenType && currentScreenObj != nullptr) {
        return;
    }
    
    currentScreenType = screen;
    
    // Load the appropriate screen
    loadCurrentScreen();
}

DisplayScreen UserInterface::getCurrentScreen() const {
    return currentScreenType;
}

void UserInterface::handleButtonAction(ButtonAction action, uint8_t buttonId) {
    if (action == ButtonAction::NONE) return;
    
    // Handle alert dismissal
    if (alertModal != nullptr && action == ButtonAction::SHORT_PRESS) {
        clearAlert();
        return;
    }
    
    // Handle screen navigation
    switch (action) {
        case ButtonAction::SHORT_PRESS:
            if (buttonId == 0) { // Button 1 - cycle screens
                DisplayScreen nextScreen = static_cast<DisplayScreen>(
                    (static_cast<int>(currentScreenType) + 1) % 4
                );
                setScreen(nextScreen);
            }
            break;
        case ButtonAction::LONG_PRESS:
            if (buttonId == 0) { // Button 1 long press - go to settings
                setScreen(DisplayScreen::SETTINGS);
            }
            break;
        default:
            break;
    }
}

void UserInterface::showAlert(const char* message, uint16_t durationMs) {
    if (message && strlen(message) > 0) {
        createAlert(message, durationMs);
    }
}

void UserInterface::clearAlert() {
    if (alertModal != nullptr) {
        lv_obj_del(alertModal);
        alertModal = nullptr;
    }
    if (alertTimer != nullptr) {
        lv_timer_del(alertTimer);
        alertTimer = nullptr;
    }
}

// Screen creation methods
lv_obj_t* UserInterface::createMainFlightScreen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Altitude display (large, center top)
    lv_obj_t* altLabel = lv_label_create(screen);
    lv_label_set_text(altLabel, "0m");
    lv_obj_set_style_text_font(altLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(altLabel, lv_color_white(), 0);
    lv_obj_align(altLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Vertical speed bar (left side)
    lv_obj_t* vsBar = lv_bar_create(screen);
    lv_obj_set_size(vsBar, 20, 100);
    lv_obj_align(vsBar, LV_ALIGN_LEFT_MID, 10, 0);
    lv_bar_set_range(vsBar, -500, 500); // -5.0 to +5.0 m/s in cm/s
    lv_bar_set_value(vsBar, 0, LV_ANIM_OFF);
    
    // Speed display (right side)
    lv_obj_t* speedLabel = lv_label_create(screen);
    lv_label_set_text(speedLabel, "0 km/h");
    lv_obj_set_style_text_font(speedLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(speedLabel, lv_color_white(), 0);
    lv_obj_align(speedLabel, LV_ALIGN_RIGHT_MID, -10, -20);
    
    // GPS status (bottom left)
    lv_obj_t* gpsLabel = lv_label_create(screen);
    lv_label_set_text(gpsLabel, "GPS: -");
    lv_obj_set_style_text_font(gpsLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(gpsLabel, lv_color_white(), 0);
    lv_obj_align(gpsLabel, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    
    // Battery indicator (bottom right)
    lv_obj_t* battBar = lv_bar_create(screen);
    lv_obj_set_size(battBar, 40, 10);
    lv_obj_align(battBar, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_bar_set_range(battBar, 0, 100);
    lv_bar_set_value(battBar, 50, LV_ANIM_OFF);
    
    return screen;
}

lv_obj_t* UserInterface::createNavigationScreen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Position display
    lv_obj_t* latLabel = lv_label_create(screen);
    lv_label_set_text(latLabel, "Lat: --");
    lv_obj_set_style_text_color(latLabel, lv_color_white(), 0);
    lv_obj_align(latLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    
    lv_obj_t* lonLabel = lv_label_create(screen);
    lv_label_set_text(lonLabel, "Lon: --");
    lv_obj_set_style_text_color(lonLabel, lv_color_white(), 0);
    lv_obj_align(lonLabel, LV_ALIGN_TOP_LEFT, 10, 30);
    
    // Speed and heading
    lv_obj_t* speedLabel = lv_label_create(screen);
    lv_label_set_text(speedLabel, "Speed: 0 km/h");
    lv_obj_set_style_text_color(speedLabel, lv_color_white(), 0);
    lv_obj_align(speedLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    
    lv_obj_t* headingLabel = lv_label_create(screen);
    lv_label_set_text(headingLabel, "Hdg: ---°");
    lv_obj_set_style_text_color(headingLabel, lv_color_white(), 0);
    lv_obj_align(headingLabel, LV_ALIGN_TOP_LEFT, 10, 70);
    
    return screen;
}

lv_obj_t* UserInterface::createSettingsScreen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Title
    lv_obj_t* titleLabel = lv_label_create(screen);
    lv_label_set_text(titleLabel, "SETTINGS");
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Volume slider
    lv_obj_t* volLabel = lv_label_create(screen);
    lv_label_set_text(volLabel, "Volume:");
    lv_obj_set_style_text_color(volLabel, lv_color_white(), 0);
    lv_obj_align(volLabel, LV_ALIGN_TOP_LEFT, 10, 40);
    
    lv_obj_t* volSlider = lv_slider_create(screen);
    lv_obj_set_width(volSlider, 100);
    lv_obj_align(volSlider, LV_ALIGN_TOP_LEFT, 70, 40);
    lv_slider_set_range(volSlider, 0, 100);
    
    return screen;
}

lv_obj_t* UserInterface::createStatusScreen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Title
    lv_obj_t* titleLabel = lv_label_create(screen);
    lv_label_set_text(titleLabel, "SYSTEM STATUS");
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // System state
    lv_obj_t* stateLabel = lv_label_create(screen);
    lv_label_set_text(stateLabel, "State: INITIALIZING");
    lv_obj_set_style_text_color(stateLabel, lv_color_white(), 0);
    lv_obj_align(stateLabel, LV_ALIGN_TOP_LEFT, 10, 40);
    
    // Sensor health
    lv_obj_t* sensorLabel = lv_label_create(screen);
    lv_label_set_text(sensorLabel, "Sensors: OK");
    lv_obj_set_style_text_color(sensorLabel, lv_color_white(), 0);
    lv_obj_align(sensorLabel, LV_ALIGN_TOP_LEFT, 10, 60);
    
    return screen;
}

lv_obj_t* UserInterface::createErrorScreen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Error title
    lv_obj_t* titleLabel = lv_label_create(screen);
    lv_label_set_text(titleLabel, "ERROR");
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Error message
    lv_obj_t* msgLabel = lv_label_create(screen);
    lv_label_set_text(msgLabel, "System Error\nPress any button");
    lv_obj_set_style_text_color(msgLabel, lv_color_white(), 0);
    lv_obj_align(msgLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_long_mode(msgLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msgLabel, 200);
    
    return screen;
}

// Screen update methods
void UserInterface::updateMainFlightScreen() {
    if (mainFlightScreen == nullptr) return;
    
    FlightData flightData = flightManager.getFusedFlightData();
    
    // Update altitude
    lv_obj_t* altLabel = lv_obj_get_child(mainFlightScreen, 0);
    if (altLabel) {
        char altText[16];
        snprintf(altText, sizeof(altText), "%.0fm", flightData.altitude);
        lv_label_set_text(altLabel, altText);
    }
    
    // Update vertical speed bar
    lv_obj_t* vsBar = lv_obj_get_child(mainFlightScreen, 1);
    if (vsBar) {
        int32_t vsValue = (int32_t)(flightData.verticalSpeed * 100); // Convert to cm/s
        lv_bar_set_value(vsBar, vsValue, LV_ANIM_OFF);
    }
    
    // Update GPS status
    lv_obj_t* gpsLabel = lv_obj_get_child(mainFlightScreen, 3);
    if (gpsLabel) {
        GPSData gpsData = flightManager.getGPSService().getGPSData();
        const char* status = gpsData.hasValidFix ? "GPS: OK" : "GPS: --";
        lv_label_set_text(gpsLabel, status);
    }
}

void UserInterface::updateNavigationScreen() {
    if (navigationScreen == nullptr) return;
    
    GPSData gpsData = flightManager.getGPSService().getGPSData();
    
    // Update position
    lv_obj_t* latLabel = lv_obj_get_child(navigationScreen, 0);
    if (latLabel) {
        char latText[32];
        snprintf(latText, sizeof(latText), "Lat: %.6f", gpsData.latitude);
        lv_label_set_text(latLabel, latText);
    }
    
    lv_obj_t* lonLabel = lv_obj_get_child(navigationScreen, 1);
    if (lonLabel) {
        char lonText[32];
        snprintf(lonText, sizeof(lonText), "Lon: %.6f", gpsData.longitude);
        lv_label_set_text(lonLabel, lonText);
    }
    
    // Update speed
    lv_obj_t* speedLabel = lv_obj_get_child(navigationScreen, 2);
    if (speedLabel) {
        char speedText[32];
        float speedKmh = gpsData.speed * 3.6f; // Convert m/s to km/h
        snprintf(speedText, sizeof(speedText), "Speed: %.1f km/h", speedKmh);
        lv_label_set_text(speedLabel, speedText);
    }
}

void UserInterface::updateSettingsScreen() {
    // Settings screen is mostly static, minimal updates needed
}

void UserInterface::updateStatusScreen() {
    if (statusScreen == nullptr) return;
    
    // Update system state
    lv_obj_t* stateLabel = lv_obj_get_child(statusScreen, 1);
    if (stateLabel) {
        SystemState state = flightManager.getSystemState();
        const char* stateText = "State: UNKNOWN";
        switch (state) {
            case SystemState::INITIALIZING: stateText = "State: INITIALIZING"; break;
            case SystemState::READY: stateText = "State: READY"; break;
            case SystemState::FLIGHT_ACTIVE: stateText = "State: FLIGHT_ACTIVE"; break;
            case SystemState::LOW_POWER: stateText = "State: LOW_POWER"; break;
            case SystemState::ERROR: stateText = "State: ERROR"; break;
        }
        lv_label_set_text(stateLabel, stateText);
    }
    
    // Update sensor health
    lv_obj_t* sensorLabel = lv_obj_get_child(statusScreen, 2);
    if (sensorLabel) {
        bool healthy = flightManager.areAllSensorsHealthy();
        lv_label_set_text(sensorLabel, healthy ? "Sensors: OK" : "Sensors: ERROR");
    }
}

void UserInterface::updateErrorScreen() {
    // Error screen is static
}

// Alert management
void UserInterface::createAlert(const char* message, uint16_t durationMs) {
    // Clear existing alert
    clearAlert();
    
    // Create modal background
    alertModal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(alertModal, 200, 80);
    lv_obj_center(alertModal);
    lv_obj_set_style_bg_color(alertModal, lv_color_hex(0x404040), 0);
    lv_obj_set_style_border_color(alertModal, lv_color_white(), 0);
    lv_obj_set_style_border_width(alertModal, 2, 0);
    
    // Create alert title
    lv_obj_t* titleLabel = lv_label_create(alertModal);
    lv_label_set_text(titleLabel, "ALERT");
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xFFFF00), 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 5);
    
    // Create message label
    lv_obj_t* msgLabel = lv_label_create(alertModal);
    lv_label_set_text(msgLabel, message);
    lv_obj_set_style_text_color(msgLabel, lv_color_white(), 0);
    lv_obj_align(msgLabel, LV_ALIGN_CENTER, 0, 10);
    lv_label_set_long_mode(msgLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msgLabel, 180);
    
    // Create timer for auto-dismiss
    alertTimer = lv_timer_create(alertTimerCallback, durationMs, this);
    lv_timer_set_repeat_count(alertTimer, 1);
}

void UserInterface::alertTimerCallback(lv_timer_t* timer) {
    UserInterface* ui = static_cast<UserInterface*>(timer->user_data);
    if (ui) {
        ui->clearAlert();
    }
}

// Helper methods
void UserInterface::loadCurrentScreen() {
    switch (currentScreenType) {
        case DisplayScreen::MAIN_FLIGHT:
            currentScreenObj = mainFlightScreen;
            break;
        case DisplayScreen::NAVIGATION:
            currentScreenObj = navigationScreen;
            break;
        case DisplayScreen::SETTINGS:
            currentScreenObj = settingsScreen;
            break;
        case DisplayScreen::STATUS:
            currentScreenObj = statusScreen;
            break;
        case DisplayScreen::ERROR:
            currentScreenObj = errorScreen;
            break;
    }
    
    if (currentScreenObj) {
        lv_scr_load(currentScreenObj);
    }
}

bool UserInterface::shouldUpdateScreen() {
    uint32_t currentTime = arduino.millis();
    uint32_t refreshRate;
    
    switch (currentScreenType) {
        case DisplayScreen::MAIN_FLIGHT:
            refreshRate = MAIN_REFRESH_RATE;
            break;
        case DisplayScreen::NAVIGATION:
            refreshRate = NAV_REFRESH_RATE;
            break;
        case DisplayScreen::SETTINGS:
            refreshRate = SETTINGS_REFRESH_RATE;
            break;
        case DisplayScreen::STATUS:
            refreshRate = STATUS_REFRESH_RATE;
            break;
        case DisplayScreen::ERROR:
            refreshRate = STATUS_REFRESH_RATE;
            break;
        default:
            refreshRate = MAIN_REFRESH_RATE;
            break;
    }
    
    return (currentTime - lastUpdate) >= refreshRate;
}

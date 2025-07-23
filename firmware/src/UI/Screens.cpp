#include "Screens.h"
#include "UserInterface.h"
#include <stdio.h>
#include <math.h>

// Helper functions for LVGL UI
namespace LVGLHelper {
    // Color definitions for flight computer UI
    static const lv_color_t COLOR_PRIMARY = lv_color_hex(0x2196F3);     // Blue
    static const lv_color_t COLOR_SUCCESS = lv_color_hex(0x4CAF50);     // Green
    static const lv_color_t COLOR_WARNING = lv_color_hex(0xFF9800);     // Orange
    static const lv_color_t COLOR_ERROR = lv_color_hex(0xF44336);       // Red
    static const lv_color_t COLOR_BACKGROUND = lv_color_hex(0x000000);  // Black
    static const lv_color_t COLOR_TEXT = lv_color_hex(0xFFFFFF);        // White
    
    void formatAltitude(float altitude, char* buffer, size_t size) {
        snprintf(buffer, size, "%.0fm", altitude);
    }

    void formatVerticalSpeed(float vspeed, char* buffer, size_t size) {
        snprintf(buffer, size, "%.1f m/s", vspeed);
    }

    void formatGPSCoordinate(double coordinate, bool isLatitude, char* buffer, size_t size) {
        char hemisphere = isLatitude ? (coordinate >= 0 ? 'N' : 'S') : (coordinate >= 0 ? 'E' : 'W');
        double absCoord = fabs(coordinate);
        int degrees = (int)absCoord;
        double minutes = (absCoord - degrees) * 60.0;
        snprintf(buffer, size, "%d°%.3f'%c", degrees, minutes, hemisphere);
    }

    void formatTime(uint32_t seconds, char* buffer, size_t size) {
        unsigned int hours = seconds / 3600;
        unsigned int minutes = (seconds % 3600) / 60;
        unsigned int secs = seconds % 60;
        snprintf(buffer, size, "%02u:%02u:%02u", hours, minutes, secs);
    }

    // Create common header with status indicators
    lv_obj_t* createHeader(lv_obj_t* parent) {
        lv_obj_t* header = lv_obj_create(parent);
        lv_obj_set_size(header, LV_PCT(100), 20);
        lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(header, COLOR_PRIMARY, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_style_radius(header, 0, 0);
        lv_obj_set_style_pad_all(header, 2, 0);
        return header;
    }

    // Create common footer with time display
    lv_obj_t* createFooter(lv_obj_t* parent) {
        lv_obj_t* footer = lv_obj_create(parent);
        lv_obj_set_size(footer, LV_PCT(100), 16);
        lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(footer, COLOR_PRIMARY, 0);
        lv_obj_set_style_border_width(footer, 0, 0);
        lv_obj_set_style_radius(footer, 0, 0);
        lv_obj_set_style_pad_all(footer, 2, 0);
        return footer;
    }
}

// MainFlightScreen LVGL implementation
void MainFlightScreen::onEnter() {
    createWidgets();
    lv_scr_load(screen);
}

void MainFlightScreen::onExit() {
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        altitudeLabel = nullptr;
        verticalSpeedBar = nullptr;
        speedLabel = nullptr;
        satelliteLabel = nullptr;
        flightStateLabel = nullptr;
        gpsStatusLabel = nullptr;
        batteryBar = nullptr;
        timeLabel = nullptr;
    }
}

void MainFlightScreen::createWidgets() {
    // Create screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, LVGLHelper::COLOR_BACKGROUND, 0);
    
    // Create header
    lv_obj_t* header = LVGLHelper::createHeader(screen);
    
    // Flight state in header
    flightStateLabel = lv_label_create(header);
    lv_obj_align(flightStateLabel, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_text_color(flightStateLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(flightStateLabel, &lv_font_montserrat_12, 0);
    
    // GPS status in header
    gpsStatusLabel = lv_label_create(header);
    lv_obj_align(gpsStatusLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(gpsStatusLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(gpsStatusLabel, &lv_font_montserrat_12, 0);
    
    // Battery bar in header
    batteryBar = lv_bar_create(header);
    lv_obj_set_size(batteryBar, 30, 12);
    lv_obj_align(batteryBar, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_bar_set_range(batteryBar, 0, 100);
    lv_obj_set_style_bg_color(batteryBar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_color(batteryBar, LVGLHelper::COLOR_SUCCESS, LV_PART_INDICATOR);
    
    // Main content area
    lv_obj_t* content = lv_obj_create(screen);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100) - 40);
    lv_obj_align(content, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 5, 0);
    
    // Large altitude display
    altitudeLabel = lv_label_create(content);
    lv_obj_align(altitudeLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_text_color(altitudeLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(altitudeLabel, &lv_font_montserrat_16, 0);
    lv_label_set_text(altitudeLabel, "ALT: 0m");
    
    // Vertical speed bar (vertical)
    verticalSpeedBar = lv_bar_create(content);
    lv_obj_set_size(verticalSpeedBar, 20, 80);
    lv_obj_align(verticalSpeedBar, LV_ALIGN_TOP_RIGHT, -10, 30);
    lv_bar_set_range(verticalSpeedBar, -500, 500);  // -5.0 to +5.0 m/s * 100
    lv_bar_set_value(verticalSpeedBar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(verticalSpeedBar, lv_color_hex(0x333333), 0);
    
    // Speed display
    speedLabel = lv_label_create(content);
    lv_obj_align(speedLabel, LV_ALIGN_CENTER, -20, 0);
    lv_obj_set_style_text_color(speedLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(speedLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(speedLabel, "SPD: 0 km/h");
    
    // Satellite count
    satelliteLabel = lv_label_create(content);
    lv_obj_align(satelliteLabel, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_text_color(satelliteLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(satelliteLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(satelliteLabel, "SAT: 0");
    
    // Create footer
    lv_obj_t* footer = LVGLHelper::createFooter(screen);
    timeLabel = lv_label_create(footer);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(timeLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_12, 0);
}

void MainFlightScreen::update() {
    updateFlightData();
}

void MainFlightScreen::updateFlightData() {
    FlightData flightData = flightManager.getFusedFlightData();
    FlightState state = flightManager.getFlightState();
    
    // Update flight state
    const char* stateText = "GND";
    switch (state) {
        case FlightState::GROUND: stateText = "GND"; break;
        case FlightState::TAKEOFF: stateText = "T/O"; break;
        case FlightState::FLYING: stateText = "FLY"; break;
        case FlightState::LANDED: stateText = "LND"; break;
    }
    lv_label_set_text(flightStateLabel, stateText);
    
    // Update GPS status
    if (flightData.gpsData.hasValidFix) {
        lv_label_set_text(gpsStatusLabel, "GPS:OK");
        lv_obj_set_style_text_color(gpsStatusLabel, LVGLHelper::COLOR_SUCCESS, 0);
    } else {
        lv_label_set_text(gpsStatusLabel, "GPS:--");
        lv_obj_set_style_text_color(gpsStatusLabel, LVGLHelper::COLOR_WARNING, 0);
    }
    
    // Update battery (mock data for now)
    lv_bar_set_value(batteryBar, 85, LV_ANIM_OFF);
    
    // Update altitude
    char buffer[32];
    LVGLHelper::formatAltitude(flightData.altitude, buffer, sizeof(buffer));
    char altText[40];
    snprintf(altText, sizeof(altText), "ALT: %s", buffer);
    lv_label_set_text(altitudeLabel, altText);
    
    // Update vertical speed bar
    int32_t vspeed_scaled = (int32_t)(flightData.verticalSpeed * 100);  // Scale to centimeters
    lv_bar_set_value(verticalSpeedBar, vspeed_scaled, LV_ANIM_OFF);
    
    // Set vario bar color based on vertical speed
    if (flightData.verticalSpeed > 0.2f) {
        lv_obj_set_style_bg_color(verticalSpeedBar, LVGLHelper::COLOR_SUCCESS, LV_PART_INDICATOR);
    } else if (flightData.verticalSpeed < -1.0f) {
        lv_obj_set_style_bg_color(verticalSpeedBar, LVGLHelper::COLOR_ERROR, LV_PART_INDICATOR);
    } else {
        lv_obj_set_style_bg_color(verticalSpeedBar, LVGLHelper::COLOR_WARNING, LV_PART_INDICATOR);
    }
    
    // Update speed
    if (flightData.gpsData.hasValidFix) {
        snprintf(buffer, sizeof(buffer), "SPD: %.0f km/h", flightData.gpsData.speed * 3.6f);
        lv_label_set_text(speedLabel, buffer);
        
        snprintf(buffer, sizeof(buffer), "SAT: %d", flightData.gpsData.satellites);
        lv_label_set_text(satelliteLabel, buffer);
    } else {
        lv_label_set_text(speedLabel, "SPD: --");
        lv_label_set_text(satelliteLabel, "SAT: --");
    }
    
    // Update time
    uint32_t seconds = arduino.millis() / 1000;
    LVGLHelper::formatTime(seconds, buffer, sizeof(buffer));
    lv_label_set_text(timeLabel, buffer);
}

void MainFlightScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    switch (buttonId) {
        case 0: // UP button
            if (action == ButtonAction::SHORT_PRESS) {
                ui.setScreen(DisplayScreen::NAVIGATION);
            }
            break;
        case 1: // DOWN button
            if (action == ButtonAction::SHORT_PRESS) {
                ui.setScreen(DisplayScreen::STATUS);
            }
            break;
        case 2: // LEFT button
            if (action == ButtonAction::LONG_PRESS) {
                ui.setScreen(DisplayScreen::SETTINGS);
            }
            break;
        case 3: // RIGHT button / SELECT
            // Could toggle units or show more info
            break;
    }
}

// NavigationScreen LVGL implementation
void NavigationScreen::onEnter() {
    createWidgets();
    lv_scr_load(screen);
}

void NavigationScreen::onExit() {
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        latitudeLabel = nullptr;
        longitudeLabel = nullptr;
        speedLabel = nullptr;
        headingLabel = nullptr;
        hdopLabel = nullptr;
        satelliteLabel = nullptr;
        compassArc = nullptr;
    }
}

void NavigationScreen::createWidgets() {
    // Create screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, LVGLHelper::COLOR_BACKGROUND, 0);
    
    // Create header
    lv_obj_t* header = LVGLHelper::createHeader(screen);
    lv_obj_t* titleLabel = lv_label_create(header);
    lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(titleLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(titleLabel, "NAVIGATION");
    
    // Main content area
    lv_obj_t* content = lv_obj_create(screen);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100) - 40);
    lv_obj_align(content, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    
    // Compass arc
    compassArc = lv_arc_create(content);
    lv_obj_set_size(compassArc, 80, 80);
    lv_obj_align(compassArc, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_arc_set_range(compassArc, 0, 360);
    lv_arc_set_bg_angles(compassArc, 0, 360);
    lv_obj_set_style_arc_color(compassArc, LVGLHelper::COLOR_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_arc_color(compassArc, LVGLHelper::COLOR_WARNING, LV_PART_INDICATOR);
    
    // Coordinates
    latitudeLabel = lv_label_create(content);
    lv_obj_align(latitudeLabel, LV_ALIGN_TOP_LEFT, 5, 10);
    lv_obj_set_style_text_color(latitudeLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(latitudeLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(latitudeLabel, "LAT: --");
    
    longitudeLabel = lv_label_create(content);
    lv_obj_align(longitudeLabel, LV_ALIGN_TOP_LEFT, 5, 30);
    lv_obj_set_style_text_color(longitudeLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(longitudeLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(longitudeLabel, "LON: --");
    
    // Speed and heading
    speedLabel = lv_label_create(content);
    lv_obj_align(speedLabel, LV_ALIGN_TOP_LEFT, 5, 60);
    lv_obj_set_style_text_color(speedLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(speedLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(speedLabel, "SPEED: --");
    
    headingLabel = lv_label_create(content);
    lv_obj_align(headingLabel, LV_ALIGN_TOP_LEFT, 5, 85);
    lv_obj_set_style_text_color(headingLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(headingLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(headingLabel, "HDG: --");
    
    // GPS quality info
    hdopLabel = lv_label_create(content);
    lv_obj_align(hdopLabel, LV_ALIGN_BOTTOM_LEFT, 5, -30);
    lv_obj_set_style_text_color(hdopLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(hdopLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(hdopLabel, "HDOP: --");
    
    satelliteLabel = lv_label_create(content);
    lv_obj_align(satelliteLabel, LV_ALIGN_BOTTOM_LEFT, 5, -10);
    lv_obj_set_style_text_color(satelliteLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(satelliteLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(satelliteLabel, "SAT: --");
    
    // Create footer
    LVGLHelper::createFooter(screen);
}

void NavigationScreen::update() {
    updateNavigationData();
}

void NavigationScreen::updateNavigationData() {
    FlightData flightData = flightManager.getFusedFlightData();
    GPSData gpsData = flightData.gpsData;
    
    if (gpsData.hasValidFix) {
        // Update coordinates
        char buffer[32];
        LVGLHelper::formatGPSCoordinate(gpsData.latitude, true, buffer, sizeof(buffer));
        char latText[40];
        snprintf(latText, sizeof(latText), "LAT: %s", buffer);
        lv_label_set_text(latitudeLabel, latText);
        
        LVGLHelper::formatGPSCoordinate(gpsData.longitude, false, buffer, sizeof(buffer));
        char lonText[40];
        snprintf(lonText, sizeof(lonText), "LON: %s", buffer);
        lv_label_set_text(longitudeLabel, lonText);
        
        // Update speed and heading
        snprintf(buffer, sizeof(buffer), "SPEED: %.0f km/h", gpsData.speed * 3.6f);
        lv_label_set_text(speedLabel, buffer);
        
        snprintf(buffer, sizeof(buffer), "HDG: %.0f°", gpsData.heading);
        lv_label_set_text(headingLabel, buffer);
        
        // Update compass
        lv_arc_set_value(compassArc, (int16_t)gpsData.heading);
        
        // Update GPS quality
        snprintf(buffer, sizeof(buffer), "HDOP: %.1f", gpsData.hdop);
        lv_label_set_text(hdopLabel, buffer);
        
        snprintf(buffer, sizeof(buffer), "SAT: %d", gpsData.satellites);
        lv_label_set_text(satelliteLabel, buffer);
    } else {
        lv_label_set_text(latitudeLabel, "LAT: WAITING...");
        lv_label_set_text(longitudeLabel, "LON: WAITING...");
        lv_label_set_text(speedLabel, "SPEED: --");
        lv_label_set_text(headingLabel, "HDG: --");
        lv_label_set_text(hdopLabel, "HDOP: --");
        lv_label_set_text(satelliteLabel, "SAT: --");
        lv_arc_set_value(compassArc, 0);
    }
}

void NavigationScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    switch (buttonId) {
        case 0: // UP button
            ui.setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
        case 1: // DOWN button
            ui.setScreen(DisplayScreen::STATUS);
            break;
        case 2: // LEFT button
            ui.setScreen(DisplayScreen::SETTINGS);
            break;
    }
}

// SettingsScreen LVGL implementation
void SettingsScreen::onEnter() {
    createWidgets();
    lv_scr_load(screen);
}

void SettingsScreen::onExit() {
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        brightnessSlider = nullptr;
        volumeSlider = nullptr;
        liftThresholdSlider = nullptr;
        sinkThresholdSlider = nullptr;
        qnhLabel = nullptr;
        loggingIntervalLabel = nullptr;
        for (int i = 0; i < 6; i++) {
            settingLabels[i] = nullptr;
        }
    }
}

void SettingsScreen::createWidgets() {
    // Create screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, LVGLHelper::COLOR_BACKGROUND, 0);
    
    // Create header
    lv_obj_t* header = LVGLHelper::createHeader(screen);
    lv_obj_t* titleLabel = lv_label_create(header);
    lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(titleLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(titleLabel, "SETTINGS");
    
    // Main content area with scrolling
    lv_obj_t* content = lv_obj_create(screen);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100) - 40);
    lv_obj_align(content, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    
    // Brightness setting
    settingLabels[0] = lv_label_create(content);
    lv_obj_set_style_text_color(settingLabels[0], LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(settingLabels[0], &lv_font_montserrat_12, 0);
    lv_label_set_text(settingLabels[0], "Brightness: 80%");
    
    brightnessSlider = lv_slider_create(content);
    lv_obj_set_width(brightnessSlider, LV_PCT(90));
    lv_slider_set_range(brightnessSlider, 10, 100);
    lv_obj_set_style_bg_color(brightnessSlider, LVGLHelper::COLOR_PRIMARY, LV_PART_INDICATOR);
    
    // Volume setting
    settingLabels[1] = lv_label_create(content);
    lv_obj_set_style_text_color(settingLabels[1], LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(settingLabels[1], &lv_font_montserrat_12, 0);
    lv_label_set_text(settingLabels[1], "Volume: 100%");
    
    volumeSlider = lv_slider_create(content);
    lv_obj_set_width(volumeSlider, LV_PCT(90));
    lv_slider_set_range(volumeSlider, 0, 100);
    lv_obj_set_style_bg_color(volumeSlider, LVGLHelper::COLOR_PRIMARY, LV_PART_INDICATOR);
    
    // Lift threshold setting
    settingLabels[2] = lv_label_create(content);
    lv_obj_set_style_text_color(settingLabels[2], LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(settingLabels[2], &lv_font_montserrat_12, 0);
    lv_label_set_text(settingLabels[2], "Lift Threshold: 0.2 m/s");
    
    liftThresholdSlider = lv_slider_create(content);
    lv_obj_set_width(liftThresholdSlider, LV_PCT(90));
    lv_slider_set_range(liftThresholdSlider, 5, 50);  // 0.05 to 0.5 m/s * 100
    lv_obj_set_style_bg_color(liftThresholdSlider, LVGLHelper::COLOR_SUCCESS, LV_PART_INDICATOR);
    
    // Sink threshold setting (read-only display for now)
    settingLabels[3] = lv_label_create(content);
    lv_obj_set_style_text_color(settingLabels[3], LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(settingLabels[3], &lv_font_montserrat_12, 0);
    lv_label_set_text(settingLabels[3], "Sink Threshold: -2.5 m/s");
    
    // QNH setting (read-only display for now)
    qnhLabel = lv_label_create(content);
    lv_obj_set_style_text_color(qnhLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(qnhLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(qnhLabel, "QNH: 1013.25 hPa");
    
    // Logging interval setting (read-only display for now)
    loggingIntervalLabel = lv_label_create(content);
    lv_obj_set_style_text_color(loggingIntervalLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(loggingIntervalLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(loggingIntervalLabel, "Log Interval: 1s");
    
    // Create footer
    LVGLHelper::createFooter(screen);
}

void SettingsScreen::update() {
    updateSettings();
}

void SettingsScreen::updateSettings() {
    SystemConfig& config = configService.getConfig();
    char buffer[64];
    
    // Update brightness
    snprintf(buffer, sizeof(buffer), "Brightness: %d%%", config.brightness);
    lv_label_set_text(settingLabels[0], buffer);
    lv_slider_set_value(brightnessSlider, config.brightness, LV_ANIM_OFF);
    
    // Update volume
    snprintf(buffer, sizeof(buffer), "Volume: %.0f%%", config.audioVolume);
    lv_label_set_text(settingLabels[1], buffer);
    lv_slider_set_value(volumeSlider, (int32_t)config.audioVolume, LV_ANIM_OFF);
    
    // Update lift threshold
    snprintf(buffer, sizeof(buffer), "Lift Threshold: %.1f m/s", config.liftThreshold);
    lv_label_set_text(settingLabels[2], buffer);
    lv_slider_set_value(liftThresholdSlider, (int32_t)(config.liftThreshold * 100), LV_ANIM_OFF);
    
    // Update other settings (read-only for now)
    snprintf(buffer, sizeof(buffer), "Sink Threshold: %.1f m/s", config.sinkThreshold);
    lv_label_set_text(settingLabels[3], buffer);
    
    snprintf(buffer, sizeof(buffer), "QNH: %.1f hPa", config.qnhPressure);
    lv_label_set_text(qnhLabel, buffer);
    
    snprintf(buffer, sizeof(buffer), "Log Interval: %ds", config.loggingInterval);
    lv_label_set_text(loggingIntervalLabel, buffer);
}

void SettingsScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    
    switch (buttonId) {
        case 0: // UP button - navigate settings or increase values
            break;
        case 1: // DOWN button - navigate settings or decrease values
            break;
        case 2: // LEFT button - back to main
            configService.saveConfig();
            ui.setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
        case 3: // RIGHT button - confirm/edit
            break;
    }
}

// StatusScreen LVGL implementation
void StatusScreen::onEnter() {
    createWidgets();
    lv_scr_load(screen);
}

void StatusScreen::onExit() {
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        systemStateLabel = nullptr;
        flightStateLabel = nullptr;
        sensorHealthLabel = nullptr;
        dataValidLabel = nullptr;
        errorLabel = nullptr;
        uptimeLabel = nullptr;
    }
}

void StatusScreen::createWidgets() {
    // Create screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, LVGLHelper::COLOR_BACKGROUND, 0);
    
    // Create header
    lv_obj_t* header = LVGLHelper::createHeader(screen);
    lv_obj_t* titleLabel = lv_label_create(header);
    lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(titleLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(titleLabel, "SYSTEM STATUS");
    
    // Main content area
    lv_obj_t* content = lv_obj_create(screen);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100) - 40);
    lv_obj_align(content, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    
    // System state
    systemStateLabel = lv_label_create(content);
    lv_obj_align(systemStateLabel, LV_ALIGN_TOP_LEFT, 5, 10);
    lv_obj_set_style_text_color(systemStateLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(systemStateLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(systemStateLabel, "System: READY");
    
    // Flight state
    flightStateLabel = lv_label_create(content);
    lv_obj_align(flightStateLabel, LV_ALIGN_TOP_LEFT, 5, 35);
    lv_obj_set_style_text_color(flightStateLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(flightStateLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(flightStateLabel, "Flight: GROUND");
    
    // Sensor health
    sensorHealthLabel = lv_label_create(content);
    lv_obj_align(sensorHealthLabel, LV_ALIGN_TOP_LEFT, 5, 60);
    lv_obj_set_style_text_color(sensorHealthLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(sensorHealthLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(sensorHealthLabel, "Sensors: OK");
    
    // Data validity
    dataValidLabel = lv_label_create(content);
    lv_obj_align(dataValidLabel, LV_ALIGN_TOP_LEFT, 5, 85);
    lv_obj_set_style_text_color(dataValidLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(dataValidLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(dataValidLabel, "Data: VALID");
    
    // Uptime
    uptimeLabel = lv_label_create(content);
    lv_obj_align(uptimeLabel, LV_ALIGN_BOTTOM_LEFT, 5, -30);
    lv_obj_set_style_text_color(uptimeLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(uptimeLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(uptimeLabel, "Uptime: 00:00:00");
    
    // Error message (initially hidden)
    errorLabel = lv_label_create(content);
    lv_obj_align(errorLabel, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_text_color(errorLabel, LVGLHelper::COLOR_ERROR, 0);
    lv_obj_set_style_text_font(errorLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(errorLabel, "");
    lv_obj_add_flag(errorLabel, LV_OBJ_FLAG_HIDDEN);
    
    // Create footer
    LVGLHelper::createFooter(screen);
}

void StatusScreen::update() {
    updateSystemStatus();
}

void StatusScreen::updateSystemStatus() {
    SystemState systemState = flightManager.getSystemState();
    FlightState flightState = flightManager.getFlightState();
    char buffer[64];
    
    // Update system state
    const char* sysStateText = "READY";
    lv_color_t sysStateColor = LVGLHelper::COLOR_SUCCESS;
    switch (systemState) {
        case SystemState::INITIALIZING: sysStateText = "INIT"; sysStateColor = LVGLHelper::COLOR_WARNING; break;
        case SystemState::READY: sysStateText = "READY"; sysStateColor = LVGLHelper::COLOR_SUCCESS; break;
        case SystemState::FLIGHT_ACTIVE: sysStateText = "FLIGHT"; sysStateColor = LVGLHelper::COLOR_PRIMARY; break;
        case SystemState::LOW_POWER: sysStateText = "LOW PWR"; sysStateColor = LVGLHelper::COLOR_WARNING; break;
        case SystemState::ERROR: sysStateText = "ERROR"; sysStateColor = LVGLHelper::COLOR_ERROR; break;
    }
    snprintf(buffer, sizeof(buffer), "System: %s", sysStateText);
    lv_label_set_text(systemStateLabel, buffer);
    lv_obj_set_style_text_color(systemStateLabel, sysStateColor, 0);
    
    // Update flight state
    const char* flightStateText = "GROUND";
    switch (flightState) {
        case FlightState::GROUND: flightStateText = "GROUND"; break;
        case FlightState::TAKEOFF: flightStateText = "TAKEOFF"; break;
        case FlightState::FLYING: flightStateText = "FLYING"; break;
        case FlightState::LANDED: flightStateText = "LANDED"; break;
    }
    snprintf(buffer, sizeof(buffer), "Flight: %s", flightStateText);
    lv_label_set_text(flightStateLabel, buffer);
    
    // Update sensor health
    bool sensorsHealthy = flightManager.areAllSensorsHealthy();
    snprintf(buffer, sizeof(buffer), "Sensors: %s", sensorsHealthy ? "OK" : "ERROR");
    lv_label_set_text(sensorHealthLabel, buffer);
    lv_obj_set_style_text_color(sensorHealthLabel, sensorsHealthy ? LVGLHelper::COLOR_SUCCESS : LVGLHelper::COLOR_ERROR, 0);
    
    // Update data validity
    bool dataValid = flightManager.isDataValid();
    snprintf(buffer, sizeof(buffer), "Data: %s", dataValid ? "VALID" : "INVALID");
    lv_label_set_text(dataValidLabel, buffer);
    lv_obj_set_style_text_color(dataValidLabel, dataValid ? LVGLHelper::COLOR_SUCCESS : LVGLHelper::COLOR_ERROR, 0);
    
    // Update uptime
    uint32_t seconds = arduino.millis() / 1000;
    LVGLHelper::formatTime(seconds, buffer, sizeof(buffer));
    char uptimeText[32];
    snprintf(uptimeText, sizeof(uptimeText), "Uptime: %s", buffer);
    lv_label_set_text(uptimeLabel, uptimeText);
    
    // Update error message if any
    const char* error = flightManager.getLastError();
    if (error[0] != '\0') {
        snprintf(buffer, sizeof(buffer), "Error: %s", error);
        lv_label_set_text(errorLabel, buffer);
        lv_obj_clear_flag(errorLabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(errorLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void StatusScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    switch (buttonId) {
        case 0: // UP button
            ui.setScreen(DisplayScreen::NAVIGATION);
            break;
        case 1: // DOWN button
            ui.setScreen(DisplayScreen::MAIN_FLIGHT);
            break;
        case 2: // LEFT button
            ui.setScreen(DisplayScreen::SETTINGS);
            break;
    }
}

// ErrorScreen LVGL implementation
void ErrorScreen::onEnter() {
    createWidgets();
    lv_scr_load(screen);
}

void ErrorScreen::onExit() {
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        errorTitleLabel = nullptr;
        errorMessageLabel = nullptr;
        instructionLabel = nullptr;
    }
}

void ErrorScreen::createWidgets() {
    // Create screen
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, LVGLHelper::COLOR_BACKGROUND, 0);
    
    // Create header
    lv_obj_t* header = LVGLHelper::createHeader(screen);
    lv_obj_set_style_bg_color(header, LVGLHelper::COLOR_ERROR, 0);
    
    // Main content area
    lv_obj_t* content = lv_obj_create(screen);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100) - 40);
    lv_obj_align(content, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    
    // Error title
    errorTitleLabel = lv_label_create(content);
    lv_obj_align(errorTitleLabel, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_color(errorTitleLabel, LVGLHelper::COLOR_ERROR, 0);
    lv_obj_set_style_text_font(errorTitleLabel, &lv_font_montserrat_16, 0);
    lv_label_set_text(errorTitleLabel, "SYSTEM ERROR");
    
    // Error message
    errorMessageLabel = lv_label_create(content);
    lv_obj_align(errorMessageLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(errorMessageLabel, LVGLHelper::COLOR_TEXT, 0);
    lv_obj_set_style_text_font(errorMessageLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(errorMessageLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(errorMessageLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(errorMessageLabel, LV_PCT(90));
    
    // Instructions
    instructionLabel = lv_label_create(content);
    lv_obj_align(instructionLabel, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_text_color(instructionLabel, LVGLHelper::COLOR_WARNING, 0);
    lv_obj_set_style_text_font(instructionLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(instructionLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(instructionLabel, "Press any button\nfor status");
    
    // Create footer
    LVGLHelper::createFooter(screen);
}

void ErrorScreen::update() {
    // Update error message
    const char* error = flightManager.getLastError();
    if (error[0] != '\0') {
        lv_label_set_text(errorMessageLabel, error);
    } else {
        lv_label_set_text(errorMessageLabel, "Unknown system error");
    }
}

void ErrorScreen::handleInput(ButtonAction action, uint8_t buttonId) {
    (void)action;
    (void)buttonId;
    // Any button press in error screen goes to status
    ui.setScreen(DisplayScreen::STATUS);
}

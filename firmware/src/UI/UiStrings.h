#pragma once

#include <cstdint>

// Every on-screen word, in English and Bahasa Indonesia. Labels stay short:
// the panel is 240 px wide and they are read in a glance.
struct UiStrings {
    const char* pageFlight;
    const char* pageThermal;
    const char* pageNav;
    const char* pageStatus;
    const char* pageSettings;

    const char* alt;
    const char* groundSpeed;
    const char* glide;
    const char* avgClimb;
    const char* track;
    const char* takeoff;
    const char* gpsAlt;
    const char* position;

    const char* system;
    const char* gps;
    const char* battery;
    const char* flightLog;
    const char* replay;
    const char* on;
    const char* off;
    const char* thermalNow;
    const char* thermalLast;
    const char* thermalNone;
    const char* gain;
    const char* timeInThermal;
    const char* gpsFix;
    const char* gpsNoFix;
    const char* satellitesFmt;   // "%u SAT"
    const char* logNotBuilt;
    const char* stateStarting;
    const char* stateReady;
    const char* stateFlying;
    const char* stateLowPower;
    const char* stateError;

    const char* language;
    const char* languageValue;
    const char* volume;
    const char* volumeLevels[4];
    const char* liftTone;
    const char* hintPages;
    const char* hintEdit;
    const char* hintEditing;

    const char* warnNoGps;
    const char* warnNoData;
    const char* warnLowBattery;
    const char* warnCriticalBattery;
    const char* errorTitle;
    const char* errorHint;
    const char* errorAdvice;
    const char* versionFmt;  // "VERSION %s"
};

inline const UiStrings& uiStrings(uint8_t language) {
    static const UiStrings en = {
        "FLIGHT", "THERMAL", "NAVIGATION", "STATUS", "SETTINGS",
        "ALT", "GS", "L/D", "AVG CLIMB", "TRACK", "TAKEOFF", "GPS ALT", "POSITION",
        "SYSTEM", "GPS", "BATTERY", "FLIGHT LOG", "REPLAY",
        "ON", "OFF", "CLIMBING", "LAST THERMAL", "NO THERMAL YET", "GAIN", "TIME", "FIX", "NO FIX", "%u SAT",
        "NOT BUILT YET", "STARTING", "READY", "IN FLIGHT", "LOW POWER", "ERROR",
        "LANGUAGE", "ENGLISH", "VOLUME", {"OFF", "LOW", "MEDIUM", "HIGH"}, "LIFT TONE",
        "UP/DOWN  PAGE", "ENTER  EDIT", "ENTER  CHANGE    CANCEL  DONE",
        "NO GPS FIX", "NO SENSOR DATA", "LOW BATTERY", "BATTERY CRITICAL",
        "SYSTEM ERROR", "CANCEL  BACK TO FLIGHT",
        "USE YOUR MAIN INSTRUMENT.\nRESTART IF THIS STAYS.",
        "BAYUPANDU  VERSION %s",
    };
    static const UiStrings id = {
        "TERBANG", "TERMAL", "NAVIGASI", "STATUS", "SETELAN",
        "TINGGI", "KEC", "L/D", "RATA2 NAIK", "ARAH", "TAKEOFF", "ALT GPS", "POSISI",
        "SISTEM", "GPS", "BATERAI", "LOG TERBANG", "REPLAY",
        "NYALA", "MATI", "NAIK", "TERMAL TERAKHIR", "BELUM ADA TERMAL", "NAIK", "WAKTU", "FIX", "BELUM FIX", "%u SAT",
        "BELUM DIBUAT", "MEMULAI", "SIAP", "TERBANG", "HEMAT DAYA", "GALAT",
        "BAHASA", "INDONESIA", "VOLUME", {"MATI", "PELAN", "SEDANG", "KERAS"}, "NADA NAIK",
        "UP/DOWN  HALAMAN", "ENTER  UBAH", "ENTER  GANTI    CANCEL  SELESAI",
        "GPS BELUM FIX", "DATA SENSOR HILANG", "BATERAI LEMAH", "BATERAI KRITIS",
        "GALAT SISTEM", "CANCEL  KEMBALI",
        "PAKAI INSTRUMEN UTAMA.\nNYALAKAN ULANG JIKA TETAP.",
        "BAYUPANDU  VERSI %s",
    };
    return language == 1 ? id : en;
}

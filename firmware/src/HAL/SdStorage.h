#pragma once

#include "IStorage.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>
#include <cstring>

// microSD card on the display's SPI bus (documentation/pinout.md), through the
// Arduino core's SD library. Each write opens, writes and closes its file, so a
// flight log is safe on the card up to the last record when power goes.
class SdStorage : public IStorage {
public:
    bool initialize() override {
        // Claim the bus with the pin map's pins before the display driver's
        // SPI.begin(), which then finds it already running.
        SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
        mounted = SD.begin(PIN_SD_CS, SPI, kClockHz);
        return isHealthy();
    }
    bool isHealthy() override { return mounted && SD.cardType() != CARD_NONE; }

    // Settings are not kept on the card yet.
    bool readConfig(SystemConfig&) override { return true; }
    bool writeConfig(const SystemConfig&) override { return true; }

    bool fileExists(const char* path) override { return mounted && SD.exists(path); }
    bool readFile(const char* path, char* buffer, size_t length) override {
        if (!mounted || length == 0) return false;
        File f = SD.open(path, FILE_READ);
        if (!f) return false;
        size_t n = f.read(reinterpret_cast<uint8_t*>(buffer), length - 1);
        buffer[n] = '\0';
        f.close();
        return true;
    }
    bool writeFile(const char* path, const char* buffer) override { return put(path, buffer, FILE_WRITE); }
    bool appendFile(const char* path, const char* buffer) override { return put(path, buffer, FILE_APPEND); }
    bool deleteFile(const char* path) override { return mounted && SD.remove(path); }
    bool makeDir(const char* path) override { return mounted && (SD.exists(path) || SD.mkdir(path)); }

private:
    static constexpr uint32_t kClockHz = 20000000;

    bool put(const char* path, const char* buffer, const char* mode) {
        if (!mounted) return false;
        File f = SD.open(path, mode);
        if (!f) return false;
        size_t len = std::strlen(buffer);
        size_t written = f.write(reinterpret_cast<const uint8_t*>(buffer), len);
        f.close();
        return written == len;
    }

    bool mounted = false;
};

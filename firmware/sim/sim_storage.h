// The SD card for the desktop simulator: a folder on disk. "/flights/x.igc" on
// the card is <root>/flights/x.igc.
#pragma once

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include "HAL/IStorage.h"

class SimStorage : public IStorage {
public:
    explicit SimStorage(std::string root) : root(std::move(root)) {}

    const std::string& getRoot() const { return root; }

    bool initialize() override {
        std::error_code ec;
        std::filesystem::create_directories(root, ec);
        return isHealthy();
    }
    bool isHealthy() override {
        std::error_code ec;
        return std::filesystem::is_directory(root, ec);
    }
    // Settings are not kept between runs, as on the device for now.
    bool readConfig(SystemConfig&) override { return true; }
    bool writeConfig(const SystemConfig&) override { return true; }

    bool fileExists(const char* path) override {
        std::error_code ec;
        return std::filesystem::exists(full(path), ec);
    }
    bool readFile(const char* path, char* buffer, size_t length) override {
        if (length == 0) return false;
        std::ifstream in(full(path), std::ios::binary);
        if (!in) return false;
        in.read(buffer, static_cast<std::streamsize>(length - 1));
        buffer[in.gcount()] = '\0';
        return true;
    }
    bool writeFile(const char* path, const char* buffer) override {
        std::ofstream out(full(path), std::ios::binary | std::ios::trunc);
        return out && out.write(buffer, std::strlen(buffer)).good();
    }
    bool appendFile(const char* path, const char* buffer) override {
        std::ofstream out(full(path), std::ios::binary | std::ios::app);
        return out && out.write(buffer, std::strlen(buffer)).good();
    }
    bool deleteFile(const char* path) override {
        std::error_code ec;
        return std::filesystem::remove(full(path), ec);
    }
    bool makeDir(const char* path) override {
        std::error_code ec;
        std::filesystem::create_directories(full(path), ec);
        return std::filesystem::is_directory(full(path), ec);
    }

private:
    std::string full(const char* path) const { return root + (path[0] == '/' ? "" : "/") + path; }
    std::string root;
};

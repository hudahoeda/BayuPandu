#pragma once

#include "IStorage.h"

class StorageImpl : public IStorage {
public:
    bool initialize() override { return true; }
    bool isHealthy() override { return true; }
    bool readConfig(SystemConfig& config) override { return true; }
    bool writeConfig(const SystemConfig& config) override { return true; }
    bool fileExists(const char* path) override { return false; }
    bool readFile(const char* path, char* buffer, size_t length) override { return true; }
    bool writeFile(const char* path, const char* buffer) override { return true; }
    bool appendFile(const char* path, const char* buffer) override { return true; }
    bool deleteFile(const char* path) override { return true; }
};

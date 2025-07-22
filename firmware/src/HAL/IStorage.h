#ifndef ISTORAGE_H
#define ISTORAGE_H

#include "Data/Types.h"
#include <cstddef> // For size_t

class IStorage {
public:
    virtual ~IStorage() = default;

    virtual bool initialize() = 0;
    virtual bool isHealthy() = 0;

    // Configuration management
    virtual bool readConfig(SystemConfig& config) = 0;
    virtual bool writeConfig(const SystemConfig& config) = 0;

    // File operations
    virtual bool fileExists(const char* path) = 0;
    virtual bool readFile(const char* path, char* buffer, size_t length) = 0;
    virtual bool writeFile(const char* path, const char* buffer) = 0;
    virtual bool appendFile(const char* path, const char* buffer) = 0;
    virtual bool deleteFile(const char* path) = 0;
};

#endif // ISTORAGE_H

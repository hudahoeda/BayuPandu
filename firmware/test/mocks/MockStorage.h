#ifndef MOCK_STORAGE_H
#define MOCK_STORAGE_H

#include "HAL/IStorage.h"
#include <map>
#include <string>
#include <vector>
#include <cstring>

class MockStorage : public IStorage {
public:
    // --- Test Control Methods ---
    void setHealthStatus(bool isHealthy) { is_healthy_ = isHealthy; }
    void injectFile(const char* path, const char* content) {
        files_[path] = content;
    }
    void setConfig(const SystemConfig& config) { config_ = config; }

    // --- IStorage Implementation ---
    bool initialize() override {
        initialize_called_ = true;
        return true;
    }

    bool isHealthy() override { return is_healthy_; }

    bool readConfig(SystemConfig& config) override {
        readConfig_call_count_++;
        if (!is_healthy_) return false;
        config = config_;
        return true;
    }

    bool writeConfig(const SystemConfig& config) override {
        writeConfig_call_count_++;
        if (!is_healthy_) return false;
        config_ = config;
        return true;
    }

    bool fileExists(const char* path) override {
        return files_.count(path) > 0;
    }

    bool readFile(const char* path, char* buffer, size_t length) override {
        readFile_call_count_++;
        if (!fileExists(path)) return false;
        
        std::string content = files_[path];
        strncpy(buffer, content.c_str(), length - 1);
        buffer[length - 1] = '\0'; // Ensure null termination
        return true;
    }

    bool writeFile(const char* path, const char* buffer) override {
        writeFile_call_count_++;
        if (!is_healthy_) return false;
        files_[path] = buffer;
        return true;
    }

    bool appendFile(const char* path, const char* buffer) override {
        appendFile_call_count_++;
        if (!is_healthy_) return false;
        files_[path] += buffer;
        return true;
    }

    bool deleteFile(const char* path) override {
        deleteFile_call_count_++;
        if (!is_healthy_) return false;
        return files_.erase(path) > 0;
    }

    // --- Inspection Methods ---
    bool wasInitializeCalled() const { return initialize_called_; }
    int getReadConfigCallCount() const { return readConfig_call_count_; }
    int getWriteConfigCallCount() const { return writeConfig_call_count_; }
    int getReadFileCallCount() const { return readFile_call_count_; }
    int getWriteFileCallCount() const { return writeFile_call_count_; }
    int getAppendFileCallCount() const { return appendFile_call_count_; }
    int getDeleteFileCallCount() const { return deleteFile_call_count_; }
    const SystemConfig& getCurrentConfig() const { return config_; }
    
    void reset() {
        is_healthy_ = true;
        initialize_called_ = false;
        readConfig_call_count_ = 0;
        writeConfig_call_count_ = 0;
        readFile_call_count_ = 0;
        writeFile_call_count_ = 0;
        appendFile_call_count_ = 0;
        deleteFile_call_count_ = 0;
        config_ = SystemConfig{};
        files_.clear();
    }

private:
    bool is_healthy_ = true;
    bool initialize_called_ = false;
    int readConfig_call_count_ = 0;
    int writeConfig_call_count_ = 0;
    int readFile_call_count_ = 0;
    int writeFile_call_count_ = 0;
    int appendFile_call_count_ = 0;
    int deleteFile_call_count_ = 0;
    SystemConfig config_{};
    std::map<std::string, std::string> files_;
};

#endif // MOCK_STORAGE_H

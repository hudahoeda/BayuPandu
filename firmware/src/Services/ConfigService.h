#pragma once

#include "HAL/IStorage.h"
#include "Data/Types.h"

class ConfigService
{
public:
    ConfigService(IStorage& storage);

    void loadConfig();
    void saveConfig();
    SystemConfig& getConfig();

private:
    IStorage& storage;
    SystemConfig config;
};

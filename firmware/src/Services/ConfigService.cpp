#include "ConfigService.h"

ConfigService::ConfigService(IStorage& storage)
    : storage(storage)
{
}

void ConfigService::loadConfig()
{
    if (!storage.readConfig(config))
    {
        // Failed to read config, so create a default one
        config = SystemConfig();
        saveConfig();
    }
}

void ConfigService::saveConfig()
{
    storage.writeConfig(config);
}

SystemConfig& ConfigService::getConfig()
{
    return config;
}

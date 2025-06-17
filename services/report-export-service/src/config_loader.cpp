#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

std::unordered_map<std::string, std::string> ConfigLoader::envMap_;

bool ConfigLoader::loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SPDLOG_ERROR("ConfigLoader: Failed to open .env file: {}", filename);
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos || line[0] == '#') continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        envMap_[key] = value;
    }

    SPDLOG_INFO("✅ .env loaded from {}", filename);
    return true;
}

std::string ConfigLoader::get(const std::string& key, const std::string& defaultValue) {
    auto it = envMap_.find(key);
    return (it != envMap_.end()) ? it->second : defaultValue;
}

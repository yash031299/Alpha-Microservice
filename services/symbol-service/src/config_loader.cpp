#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <spdlog/spdlog.h>

std::unordered_map<std::string, std::string> ConfigLoader::envMap_;

bool ConfigLoader::loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SPDLOG_ERROR("❌ Failed to open .env file: {}", filename);
        return false;
    }

    std::string line;
    while (getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos || line[0] == '#') continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

        envMap_[key] = value;
    }

    SPDLOG_INFO("✅ .env loaded with {} variables", envMap_.size());
    return true;
}

std::string ConfigLoader::get(const std::string& key, const std::string& defaultValue) {
    auto it = envMap_.find(key);
    return (it != envMap_.end()) ? it->second : defaultValue;
}

int ConfigLoader::getInt(const std::string& key, int defaultValue) {
    auto val = get(key);
    if (val.empty()) return defaultValue;
    try {
        return std::stoi(val);
    } catch (...) {
        return defaultValue;
    }
}

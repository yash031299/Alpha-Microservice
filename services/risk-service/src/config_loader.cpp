#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

std::unordered_map<std::string, std::string> ConfigLoader::config_;

void ConfigLoader::loadEnv(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        auto eqPos = line.find('=');
        if (eqPos == std::string::npos || line[0] == '#') continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        // Trim whitespace
        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        val.erase(std::remove_if(val.begin(), val.end(), ::isspace), val.end());

        config_[key] = val;
    }
}

std::string ConfigLoader::getEnv(const std::string& key, const std::string& defaultVal) {
    const char* fromEnv = std::getenv(key.c_str());
    if (fromEnv) return std::string(fromEnv);

    if (config_.count(key)) return config_[key];
    return defaultVal;
}

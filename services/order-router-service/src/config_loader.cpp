#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <spdlog/spdlog.h>

ConfigLoader::ConfigLoader(const std::string& filename)
    : filename_(filename) {}

void ConfigLoader::load() {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        SPDLOG_WARN("⚠️ Could not open .env file: {}", filename_);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove comments and trim whitespace
        auto pos = line.find('#');
        if (pos != std::string::npos) line = line.substr(0, pos);

        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty()) continue;

        auto delimiter = line.find('=');
        if (delimiter != std::string::npos) {
            std::string key = line.substr(0, delimiter);
            std::string val = line.substr(delimiter + 1);
            config_[key] = val;
        }
    }

    SPDLOG_INFO("✅ Loaded .env variables from {}", filename_);
}

std::string ConfigLoader::get(const std::string& key, const std::string& defaultVal) const {
    auto it = config_.find(key);
    return it != config_.end() ? it->second : defaultVal;
}

int ConfigLoader::getInt(const std::string& key, int defaultVal) const {
    try {
        return std::stoi(get(key, std::to_string(defaultVal)));
    } catch (...) {
        return defaultVal;
    }
}

double ConfigLoader::getDouble(const std::string& key, double defaultVal) const {
    try {
        return std::stod(get(key, std::to_string(defaultVal)));
    } catch (...) {
        return defaultVal;
    }
}

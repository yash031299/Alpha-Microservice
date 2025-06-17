#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <spdlog/spdlog.h>

std::unordered_map<std::string, std::string> ConfigLoader::config;

void ConfigLoader::loadEnv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        SPDLOG_ERROR("Failed to open .env file: {}", path);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            key = line.substr(0, eq);
            value = line.substr(eq + 1);
            config[key] = value;
            setenv(key.c_str(), value.c_str(), 1);
        }
    }

    SPDLOG_INFO("✅ Environment variables loaded from {}", path);
}

std::string ConfigLoader::getEnv(const std::string& key, const std::string& fallback) {
    auto it = config.find(key);
    return (it != config.end()) ? it->second : fallback;
}

int ConfigLoader::getEnvInt(const std::string& key, int fallback) {
    auto it = config.find(key);
    if (it != config.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

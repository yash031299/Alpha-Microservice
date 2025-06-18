#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

std::unordered_map<std::string, std::string> ConfigLoader::config;

void ConfigLoader::load(const std::string& filepath) {
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("❌ Could not open .env file");
    }
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        config[key] = value;
    }
}

std::string ConfigLoader::get(const std::string& key, const std::string& default_val) {
    return config.count(key) ? config[key] : default_val;
}

#include "env_loader.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>

std::unordered_map<std::string, std::string> EnvLoader::env_;

void EnvLoader::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open .env file at " + path);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);
        env_[key] = val;
        setenv(key.c_str(), val.c_str(), 1);  // Also update process env
    }
}

std::string EnvLoader::get(const std::string& key, const std::string& defaultVal) {
    const char* val = std::getenv(key.c_str());
    if (val) return std::string(val);
    return defaultVal;
}


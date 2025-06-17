#include "config_loader.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>

void ConfigLoader::loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto sep = line.find('=');
        if (sep != std::string::npos) {
            auto key = line.substr(0, sep);
            auto value = line.substr(sep + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

std::string ConfigLoader::getEnv(const std::string& key, const std::string& fallback) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : fallback;
}

int ConfigLoader::getIntEnv(const std::string& key, int fallback) {
    const char* val = std::getenv(key.c_str());
    return val ? std::atoi(val) : fallback;
}

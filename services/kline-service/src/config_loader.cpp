#include "config_loader.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>

void ConfigLoader::loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string key, val;

        if (std::getline(iss, key, '=') && std::getline(iss, val)) {
            setenv(key.c_str(), val.c_str(), 1);
        }
    }
}

std::string ConfigLoader::getEnv(const std::string& key, const std::string& defaultValue) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultValue;
}

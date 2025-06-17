#include "config_loader.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

std::unordered_map<std::string, std::string> ConfigLoader::envMap_;

void ConfigLoader::loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(std::getline(iss, key, '='), value)) {
            envMap_[key] = value;
            setenv(key.c_str(), value.c_str(), 1);  // Set as real env var too
        }
    }
}

std::string ConfigLoader::getEnv(const std::string& key, const std::string& defaultVal) {
    auto it = envMap_.find(key);
    if (it != envMap_.end()) return it->second;
    const char* env = std::getenv(key.c_str());
    return env ? std::string(env) : defaultVal;
}

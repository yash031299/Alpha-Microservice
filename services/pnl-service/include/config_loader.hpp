#pragma once
#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    static void loadEnv(const std::string& filepath);
    static std::string getEnv(const std::string& key, const std::string& defaultVal = "");

private:
    static std::unordered_map<std::string, std::string> config_;
};

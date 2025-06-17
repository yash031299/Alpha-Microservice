#pragma once
#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    static void loadEnv(const std::string& path = ".env");
    static std::string getEnv(const std::string& key, const std::string& fallback = "");
    static int getEnvInt(const std::string& key, int fallback = 0);

private:
    static std::unordered_map<std::string, std::string> config;
};

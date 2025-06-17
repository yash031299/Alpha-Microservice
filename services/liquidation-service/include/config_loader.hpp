#pragma once
#include <string>

class ConfigLoader {
public:
    static void loadEnv(const std::string& filename = ".env");
    static std::string getEnv(const std::string& key, const std::string& fallback = "");
    static int getIntEnv(const std::string& key, int fallback = 0);
};

#pragma once
#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    static void load(const std::string& filepath = ".env");
    static std::string get(const std::string& key, const std::string& default_val = "");

private:
    static std::unordered_map<std::string, std::string> config;
};

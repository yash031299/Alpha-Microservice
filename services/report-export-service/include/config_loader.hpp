#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    static bool loadEnv(const std::string& filename = ".env");
    static std::string get(const std::string& key, const std::string& defaultValue = "");

private:
    static std::unordered_map<std::string, std::string> envMap_;
};

#endif // CONFIG_LOADER_HPP

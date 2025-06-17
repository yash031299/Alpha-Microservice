#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    static void loadEnv(const std::string& filename = ".env");
    static std::string getEnv(const std::string& key, const std::string& defaultVal = "");

private:
    static std::unordered_map<std::string, std::string> envMap_;
};

#endif // CONFIG_LOADER_HPP

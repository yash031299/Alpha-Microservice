#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    static bool loadEnv(const std::string& filename);
    static std::string get(const std::string& key, const std::string& defaultValue = "");
    static int getInt(const std::string& key, int defaultValue = 0);

private:
    static std::unordered_map<std::string, std::string> envMap_;
};

#endif // CONFIG_LOADER_HPP

#ifndef ENV_LOADER_HPP
#define ENV_LOADER_HPP

#include <string>
#include <unordered_map>

class EnvLoader {
public:
    static void load(const std::string& path = ".env");
    static std::string get(const std::string& key, const std::string& defaultVal = "");
private:
    static std::unordered_map<std::string, std::string> env_;
};

#endif

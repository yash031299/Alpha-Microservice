#pragma once
#include <string>
#include <unordered_map>

class ConfigLoader {
public:
    explicit ConfigLoader(const std::string& filename = ".env");
    void load();

    std::string get(const std::string& key, const std::string& defaultVal = "") const;
    int getInt(const std::string& key, int defaultVal = 0) const;
    double getDouble(const std::string& key, double defaultVal = 0.0) const;

private:
    std::string filename_;
    std::unordered_map<std::string, std::string> config_;

};

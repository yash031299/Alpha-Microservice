#ifndef ENV_LOADER_HPP
#define ENV_LOADER_HPP

#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <spdlog/spdlog.h>

class EnvLoader {
public:
    static void load(const std::string& filename = ".env") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            SPDLOG_WARN(".env file not found: {}", filename);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Remove comments
            auto comment = line.find('#');
            if (comment != std::string::npos) line = line.substr(0, comment);
            if (line.empty()) continue;

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = trim(line.substr(0, eq));
            std::string value = trim(line.substr(eq + 1));

            setenv(key.c_str(), value.c_str(), 1);
        }

        SPDLOG_INFO("✅ Environment variables loaded from .env");
    }

private:
    static std::string trim(const std::string& s) {
        const char* ws = " \t\n\r\f\v";
        size_t start = s.find_first_not_of(ws);
        size_t end = s.find_last_not_of(ws);
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
};

#endif // ENV_LOADER_HPP

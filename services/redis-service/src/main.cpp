#include "redis_pool_singleton.hpp"
#include "health_check.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void loadEnv(const std::string& filename = ".env") {
    std::ifstream file(filename);
    if (!file.is_open()) {
        spdlog::warn("No .env file found. Proceeding with default values.");
        return;
    }

    std::string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t delimPos = line.find('=');
        if (delimPos == std::string::npos) continue;

        std::string key = line.substr(0, delimPos);
        std::string val = line.substr(delimPos + 1);
        setenv(key.c_str(), val.c_str(), 1);
    }

    spdlog::info("✅ .env loaded");
}

void setupLogging() {
    try {
        auto logger = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    } catch (const std::exception& e) {
        std::cerr << "Logging setup failed: " << e.what() << std::endl;
    }
}

int main() {
    try {
        setupLogging();
        loadEnv();

        const char* host = std::getenv("REDIS_HOST");
        const char* port = std::getenv("REDIS_PORT");
        const char* poolSizeStr = std::getenv("REDIS_POOL_SIZE");

        std::string redisHost = host ? host : "127.0.0.1";
        int redisPort = port ? std::stoi(port) : 6379;
        int poolSize = poolSizeStr ? std::stoi(poolSizeStr) : 4;

        RedisPoolSingleton::init(redisHost, redisPort, poolSize);

        int healthPort = std::getenv("HEALTH_PORT") ? std::stoi(std::getenv("HEALTH_PORT")) : 8084;
        std::thread healthThread([=] { startHealthCheckServer(healthPort); });
        healthThread.detach();

        spdlog::info("🟢 redis-service is running.");
        std::promise<void>().get_future().wait();  // keep alive forever

    } catch (const std::exception& e) {
        spdlog::critical("Unhandled exception in redis-service: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        spdlog::critical("Unknown fatal error in redis-service.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include "health_check.hpp"
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>
#include <string>
#include <csignal>


void runHealthMonitor(const std::string& serviceName, int intervalSeconds) {
    std::thread([serviceName, intervalSeconds]() {
        try {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
                SPDLOG_INFO("[{}] service is alive", serviceName);
            }
        } catch (const std::exception& e) {
            SPDLOG_ERROR("Health monitor crashed for {}: {}", serviceName, e.what());
        } catch (...) {
            SPDLOG_ERROR("Unknown error in health monitor for {}", serviceName);
        }
    }).detach();
}

#include "utils.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include <chrono>
#include <iostream>

void utils::initLogging() {
    try {
        auto logger = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
    }
}

void utils::backoff(int ms) {
    SPDLOG_WARN("⏳ Backing off for {} ms", ms);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#include "redis_stream_listener.hpp"
#include "utils.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    // Initialize spdlog with color output and standard formatting
    utils::initLogging();

    SPDLOG_INFO("🚀 Starting Stream Service...");

    try {
        // Create and run the Redis tick stream listener
        RedisStreamListener listener;
        listener.run();
    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("❌ Fatal error in stream service: {}", ex.what());
    } catch (...) {
        SPDLOG_CRITICAL("❌ Unknown fatal error occurred.");
    }

    SPDLOG_INFO("🧹 Stream Service exited cleanly.");
    return 0;
}

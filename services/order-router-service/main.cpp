#include "order_router.hpp"
#include <csignal>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include "grpc_client.hpp"
#include "match_engine_client.hpp"
#include "rms_client.hpp"

std::atomic<bool> stopFlag = false;

void signalHandler(int signal) {
    SPDLOG_WARN("🛑 Signal received: {}. Shutting down...", signal);
    stopFlag = true;
}

int main() {
    SPDLOG_INFO("🚀 Starting order-router-service...");

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        std::string redis_host = std::getenv("REDIS_HOST") ? std::getenv("REDIS_HOST") : "127.0.0.1";
        int redis_port = std::getenv("REDIS_PORT") ? std::stoi(std::getenv("REDIS_PORT")) : 6379;

        auto matchClient = std::make_shared<MatchEngineClient>("localhost:50061");  // or use ENV
        auto rmsClient = std::make_shared<RMSClient>("localhost:50052");           // or use ENV

        OrderRouterService router(redis_host, redis_port, matchClient, rmsClient);
        router.start();

        while (!stopFlag) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        router.stop();
        SPDLOG_INFO("✅ order-router-service shut down cleanly.");
        return 0;

    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("💥 Fatal error: {}", ex.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("💥 Unknown fatal error in main()");
        return EXIT_FAILURE;
    }
}

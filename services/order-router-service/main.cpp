#include "order_router.hpp"
#include "grpc_client.hpp"
#include "redis_client.hpp"
#include <csignal>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

std::atomic<bool> stopFlag = false;

void signalHandler(int signal) {
    SPDLOG_WARN("🛑 Signal received: {}. Shutting down...", signal);
    stopFlag = true;
}

int main() {
    SPDLOG_INFO("🚀 Starting order-router-service...");

    // Step 1: Register signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // Step 2: Initialize Redis client
        auto redisClient = std::make_shared<RedisClient>("127.0.0.1", 6379);
        if (!redisClient->isConnected()) {
            SPDLOG_CRITICAL("❌ Failed to connect to Redis");
            return EXIT_FAILURE;
        }

        // Step 3: Initialize gRPC clients
        auto matchEngineClient = std::make_shared<MatchEngineClient>("localhost:50051");
        auto rmsClient = std::make_shared<RMSClient>("localhost:50052");

        // Step 4: Start order router
        OrderRouter router(redisClient, matchEngineClient, rmsClient);
        router.start();

        // Step 5: Run until shutdown signal
        while (!stopFlag) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Step 6: Graceful shutdown
        router.stop();
        SPDLOG_INFO("✅ order-router-service shut down cleanly.");
        return 0;

    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("💥 Unhandled exception in main(): {}", ex.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("💥 Unknown fatal error in main()");
        return EXIT_FAILURE;
    }
}

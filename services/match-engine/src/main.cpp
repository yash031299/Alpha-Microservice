#include "matcher.hpp"
#include "trade_publisher.hpp"
#include "redis_listener.hpp"
#include "grpc_server.hpp"
#include "env_loader.hpp"
#include "health_check.hpp"

#include <csignal>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>

std::atomic<bool> running = true;

std::unique_ptr<RedisListener> redisListener;
std::unique_ptr<HealthCheckServer> health;
std::unique_ptr<Matcher> matcher;
std::unique_ptr<TradePublisher> publisher;

void signalHandler(int signal) {
    SPDLOG_WARN("🔌 Signal received: {}. Initiating shutdown...", signal);
    running = false;
}

int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    SPDLOG_INFO("🚀 Starting match-engine...");

    try {
        // Load environment variables
        EnvLoader::load(".env");

        std::string grpcPort = EnvLoader::get("GRPC_PORT", "50051");
        std::string redisHost = EnvLoader::get("REDIS_HOST", "127.0.0.1");
        int redisPort = std::stoi(EnvLoader::get("REDIS_PORT", "6379"));
        std::string redisStream = EnvLoader::get("REDIS_ORDER_STREAM", "ORDER_STREAM");

        matcher = std::make_unique<Matcher>();
        publisher = std::make_unique<TradePublisher>();
        redisListener = std::make_unique<RedisListener>(redisHost, redisPort, redisStream, *matcher);
        health = std::make_unique<HealthCheckServer>(8081);

        // Start services
        health->start();
        redisListener->start();

        // Setup signal handler for graceful shutdown
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // Start gRPC server (blocking)
        SPDLOG_INFO("🌐 gRPC server starting on 0.0.0.0:{} ...", grpcPort);
        runGrpcServer(*matcher, *publisher, "0.0.0.0:" + grpcPort);
        SPDLOG_INFO("✅ gRPC server running.");

        // Keep process alive until signal received
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        
        // Shutdown flow
        SPDLOG_INFO("🛑 Shutting down services...");
        redisListener->stop();
        health->stop();
        SPDLOG_INFO("✅ Match-engine shutdown complete.");
        return 0;

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("💥 Fatal error: {}", e.what());
        return 1;
    } catch (...) {
        SPDLOG_CRITICAL("💥 Unknown fatal error");
        return 2;
    }
}

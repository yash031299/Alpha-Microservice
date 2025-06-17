#include "grpc_user_server.hpp"
#include "redis_safe_client.hpp"
#include "user_wallet_store.hpp"
#include "user_metadata_store.hpp"
#include "margin_listener.hpp"
#include "scheduler.hpp"
#include "env_loader.hpp"
#include <spdlog/spdlog.h>
#include <memory>
#include <thread>
#include <csignal>
#include <atomic>
#include <cstdlib>
#include <spdlog/sinks/stdout_color_sinks.h>


extern void runHealthCheck(const std::string& serviceName, int intervalSeconds);

std::atomic<bool> running{true};

void signalHandler(int) {
    running = false;
}

int main() {
    try {
        auto logger = spdlog::stdout_color_mt("user-service");
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

        SPDLOG_INFO("🚀 Bootstrapping user-service...");
        EnvLoader::load(".env");

        std::string redisHost = std::getenv("REDIS_HOST") ? std::getenv("REDIS_HOST") : "127.0.0.1";
        int redisPort = std::getenv("REDIS_PORT") ? std::stoi(std::getenv("REDIS_PORT")) : 6379;
        int flushInterval = std::getenv("WALLET_FLUSH_INTERVAL") ? std::stoi(std::getenv("WALLET_FLUSH_INTERVAL")) : 5;
        std::string grpcPort = std::getenv("GRPC_PORT") ? std::getenv("GRPC_PORT") : "50052";

        auto redis = std::make_shared<RedisSafeClient>(redisHost, redisPort);
        if (!redis->isConnected()) {
            SPDLOG_CRITICAL("❌ Cannot proceed: Redis connection failed.");
            return 1;
        }

        auto walletStore   = std::make_shared<UserWalletStore>(redis);
        auto metadataStore = std::make_shared<UserMetadataStore>(redis);

        walletStore->initializeFromRedis();
        metadataStore->initializeFromRedis();

        MarginEventListener listener(redis, walletStore);
        listener.start();

        WalletFlushScheduler scheduler(walletStore, flushInterval);
        scheduler.start();

        runHealthCheck("user-service", 15);

        // Setup signal handler for graceful shutdown
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // Run gRPC server in separate thread
        std::string serverAddress = "0.0.0.0:" + grpcPort;
        std::thread grpcThread([walletStore, metadataStore, serverAddress]() {
            RunGrpcServer(walletStore, metadataStore, serverAddress);
        });

        SPDLOG_INFO("✅ user-service is up and running.");

        // Wait for termination signal
        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        SPDLOG_INFO("Shutdown signal received, stopping services...");

        listener.stop();
        scheduler.stop();

        // Note: gRPC server shuts down when RunGrpcServer returns, implement graceful shutdown there if needed

        if (grpcThread.joinable()) {
            grpcThread.join();
        }

        SPDLOG_INFO("user-service stopped gracefully.");

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("🔥 Fatal exception in user-service: {}", e.what());
        return 1;
    } catch (...) {
        SPDLOG_CRITICAL("🔥 Unknown fatal exception occurred in user-service");
        return 1;
    }

    return 0;
}

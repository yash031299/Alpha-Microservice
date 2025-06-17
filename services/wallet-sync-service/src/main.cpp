#include "redis_listener.hpp"
#include "config_loader.hpp"
#include <spdlog/spdlog.h>
#include <thread>
#include <csignal>
#include <atomic>
#include "wallet_store.hpp"

WalletStore walletStore;

int startHealthCheckServer();

std::shared_ptr<RedisListener> listener;
std::atomic<bool> isShuttingDown{false};

void handleSignal(int signal) {
    SPDLOG_WARN("⚠️ Signal {} received. Shutting down...", signal);
    isShuttingDown = true;

    if (listener) {
        listener->stop();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    exit(0);
}

int main() {
    SPDLOG_INFO("🚀 [wallet-sync-service] Bootstrapping...");

    try {
        ConfigLoader::loadEnv(".env");

        // Attach graceful shutdown handlers
        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        // Start Redis Listener
        listener = std::make_shared<RedisListener>();
        listener->start();

        // Start health check server (Boost.Beast)
        startHealthCheckServer();

        SPDLOG_INFO("✅ wallet-sync-service is up and running.");

        // Block main thread
        while (!isShuttingDown) {
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("💥 Fatal exception in main: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("💥 Unknown fatal exception in main.");
        return EXIT_FAILURE;
    }

    return 0;
}

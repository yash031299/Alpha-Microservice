#include "config_loader.hpp"
#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>
#include <spdlog/spdlog.h>
#include "utils/metrics.hpp"
#include "http_health_server.hpp"
#include "redis_connection_pool.hpp"
#include "redis_user_client.hpp"
#include "liquidation_controller.hpp"
#include "liquidation_strategy.hpp"

std::atomic<bool> shutdown_requested = false;

void signalHandler(int signum) {
    SPDLOG_WARN("🔌 Signal received: {}. Initiating shutdown...", signum);
    shutdown_requested = true;
}

int main() {
    try {
        SPDLOG_INFO("🚀 Starting liquidation-service...");

        // Signal handling
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // Load .env
        try {
            ConfigLoader::loadEnv(".env");
        } catch (const std::exception& e) {
            SPDLOG_CRITICAL("❌ Failed to load .env: {}", e.what());
            return EXIT_FAILURE;
        }

        // Health check server
        int healthPort = ConfigLoader::getIntEnv("HEALTH_PORT", 8081);
        try {
            startHealthServer(healthPort);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("❌ Failed to start health server: {}", e.what());
        }

        // Redis Connection Pool
        std::shared_ptr<RedisConnectionPool> redisPool;
        try {
            std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
            int redisPort = ConfigLoader::getIntEnv("REDIS_PORT", 6379);
            int poolSize = ConfigLoader::getIntEnv("REDIS_POOL_SIZE", 4);
            redisPool = std::make_shared<RedisConnectionPool>(redisHost, redisPort, poolSize);
        } catch (const std::exception& e) {
            SPDLOG_CRITICAL("❌ Failed to initialize Redis connection pool: {}", e.what());
            return EXIT_FAILURE;
        }

        // Metrics
        try {
            int metricsPort = ConfigLoader::getIntEnv("METRICS_PORT", 9100);
            Metrics::init(metricsPort);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("⚠️ Metrics init failed: {}", e.what());
        }

        // Redis Client + Strategy + Controller
        try {
            auto redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
            int redisPort = ConfigLoader::getIntEnv("REDIS_PORT", 6379);

            auto redisUserClient = std::make_shared<RedisUserClient>(redisPool, redisHost, redisPort);
            auto strategy = createDefaultStrategy();

            int threadCount = ConfigLoader::getIntEnv("THREAD_COUNT", 4);
            LiquidationController controller(redisPool, redisUserClient, strategy, threadCount);
            controller.start();

            // Graceful wait
            while (!shutdown_requested) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            controller.stop();
            SPDLOG_INFO("🧼 Shutdown complete.");
            return EXIT_SUCCESS;

        } catch (const std::exception& e) {
            SPDLOG_CRITICAL("🔥 Failed to start controller: {}", e.what());
            return EXIT_FAILURE;
        }

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("❌ Uncaught exception in main: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("❌ Unknown fatal error occurred in main()");
        return EXIT_FAILURE;
    }
}

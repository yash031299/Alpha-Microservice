#include "ledger.hpp"
#include "redis_listener.hpp"
#include "settlement_engine.hpp"
#include "metrics_server.hpp"
#include "grpc_interface.hpp"

#include <boost/asio.hpp>
#include <csignal>
#include <memory>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <thread>
#include <atomic>

std::shared_ptr<SettlementEngine> global_settlement;
std::unique_ptr<AccountingGRPCServer> grpc_server_instance;
std::unique_ptr<RedisListener> redis_listener;
std::shared_ptr<MetricsServer> metrics;
boost::asio::io_context io;
std::vector<std::thread> io_threads;
std::thread grpc_thread;

void handleShutdown(int signum) {
    SPDLOG_WARN("🛑 Signal {} received — initiating graceful shutdown...", signum);

    if (global_settlement) {
        SPDLOG_INFO("📥 Flushing final snapshot before exit...");
        global_settlement->performSettlement();
    }

    if (metrics) {
        SPDLOG_INFO("🛑 Stopping metrics server...");
        metrics->stop();
    }

    if (redis_listener) {
        SPDLOG_INFO("🛑 Stopping Redis listener...");
        redis_listener->stop();
    }

    if (grpc_server_instance) {
        SPDLOG_INFO("🛑 Stopping gRPC server...");
        grpc_server_instance->shutdown();
    }

    io.stop();
}


int main() {
    SPDLOG_INFO("🚀 Starting accounting-service...");

    try {
        std::signal(SIGINT, handleShutdown);
        std::signal(SIGTERM, handleShutdown);

        const char* grpc_host = std::getenv("GRPC_HOST") ?: "0.0.0.0";
        const char* grpc_port = std::getenv("GRPC_PORT") ?: "6008";
        const char* interval_env = std::getenv("SNAPSHOT_INTERVAL_SEC") ?: "60";
        const char* redis_flag = std::getenv("ENABLE_REDIS_EVENTS");

        std::string grpc_address = std::string(grpc_host) + ":" + std::string(grpc_port);
        int snapshot_interval = std::stoi(interval_env);
        bool enable_redis = redis_flag && std::string(redis_flag) == "true";

        auto ledger = std::make_shared<Ledger>();

        if (enable_redis) {
            redis_listener = std::make_unique<RedisListener>("localhost", 6379, ledger);
            redis_listener->start();
            SPDLOG_INFO("📡 Redis listener active");
        }

        global_settlement = std::make_shared<SettlementEngine>(io, ledger, snapshot_interval);
        global_settlement->start();

        metrics = std::make_shared<MetricsServer>(ledger, 8081);
        metrics->start();

        grpc_server_instance = std::make_unique<AccountingGRPCServer>(ledger);
        grpc_thread = std::thread([&]() {
            grpc_server_instance->run(grpc_address);  // 🔁 will unblock when shutdown() is called
        });

        const int thread_count = std::thread::hardware_concurrency();
        for (int i = 0; i < thread_count; ++i)
            io_threads.emplace_back([] { io.run(); });

        if (grpc_thread.joinable()) {
            grpc_thread.join();
        }
        for (auto& t : io_threads) {
            if (t.joinable()) t.join();
        }

    }
    catch (const std::exception& ex) {
        SPDLOG_CRITICAL("🔥 Startup failed: {}", ex.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        SPDLOG_CRITICAL("🔥 Unknown error occurred during startup.");
        return EXIT_FAILURE;
    }

    return 0;
}

#include "orderbook_manager.hpp"
#include "snapshot_generator.hpp"
#include "config_loader.hpp"
#include "redis_publisher.hpp"
#include "grpc_server.hpp"
#include "health_check.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <thread>
#include <chrono>
#include <memory>


std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}



int main() {
    try {
        // Logger setup
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("symbol-service", sink);
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

        SPDLOG_INFO("🚀 Starting symbol-service...");

        // Load config
        ConfigLoader::loadEnv(".env");
        int interval = std::stoi(ConfigLoader::get("SNAPSHOT_INTERVAL_SEC", "5"));
        std::string redisHost = ConfigLoader::get("REDIS_HOST", "localhost");
        int redisPort = std::stoi(ConfigLoader::get("REDIS_PORT", "6379"));
        std::string grpcAddr = ConfigLoader::get("GRPC_PORT", "50061");
        std::string bindAddr = "0.0.0.0:" + grpcAddr;

        auto symbols = split(ConfigLoader::get("SYMBOLS"), ',');
        SPDLOG_INFO("✅ Loaded {} symbols: {}", symbols.size(), ConfigLoader::get("SYMBOLS"));

        // Init core managers
        auto book = std::make_shared<OrderBookManager>();
        RedisPublisher redis(redisHost, redisPort);

        // Add initial mock orders
        for (const auto& sym : symbols) {
            book->addOrder(sym, Order{sym + "_buy", 25980.0, 1.0, true});
            book->addOrder(sym, Order{sym + "_sell", 25990.0, 1.0, false});
        }

        // Start gRPC server in background thread
        std::thread([book, bindAddr]() {
            auto service = std::make_shared<SymbolSnapshotServiceImpl>(book);
            runGrpcServer(service, bindAddr);
        }).detach();

        // Start health monitor
        runHealthMonitor("symbol-service", interval);


        // Periodically publish snapshots to Redis
        while (true) {
            for (const auto& sym : symbols) {
                auto levels = book->getTopOfBook(sym, 1);
                std::string snapshot = SnapshotGenerator::generateJsonL1(sym, levels);
                redis.publishSnapshot(sym, snapshot);
            }
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

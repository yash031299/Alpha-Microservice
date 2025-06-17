#include "grpc_interface.hpp"
#include "config_loader.hpp"
#include "health_check.hpp"
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include <hiredis/hiredis.h>
#include <csignal>
#include <atomic>
#include <chrono>
#include <httplib.h>
#include <rapidjson/document.h>

std::atomic<bool> keepRunning(true);

void shutdownHandler(int signum) {
    SPDLOG_WARN("🔌 Signal received: {}. Shutting down gracefully...", signum);
    keepRunning = false;
}

void streamTradeListener(PnLServiceImpl* service, const std::string& redisHost, int redisPort) {
    SPDLOG_INFO("🧠 Starting trade stream listener on Redis {}:{}...", redisHost, redisPort);
    redisContext* redis = redisConnect(redisHost.c_str(), redisPort);
    if (!redis || redis->err) {
        SPDLOG_ERROR("❌ Redis connection failed: {}", redis ? redis->errstr : "null");
        return;
    }

    std::string last_id = "0";

    while (keepRunning) {
        try {
            redisReply* reply = (redisReply*)redisCommand(redis, "XREAD COUNT 1 BLOCK 5000 STREAMS STREAM:TRADE %s", last_id.c_str());
            if (!reply || reply->type != REDIS_REPLY_ARRAY) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            if (reply->element[0]->elements >= 2) {
                auto entries = reply->element[0]->element[1];
                for (size_t i = 0; i < entries->elements; ++i) {
                    auto entry = entries->element[i];
                    std::string entry_id = entry->element[0]->str;
                    std::string jsonTrade = entry->element[1]->element[1]->str;
                    last_id = entry_id;

                    rapidjson::Document doc;
                    doc.Parse(jsonTrade.c_str());
                    if (doc.HasParseError()) {
                        SPDLOG_ERROR("⚠️ JSON parse error: {}", jsonTrade);
                        continue;
                    }

                    std::string userId = doc["user"].GetString();
                    std::string symbol = doc["symbol"].GetString();
                    double price = doc["price"].GetDouble();
                    double qty = doc["qty"].GetDouble();
                    std::string side = doc["side"].GetString();

                    auto user = service->getOrCreateUser(userId);

                    // 👇 Metrics before logic
                    incrementLTPCount();
                    incrementTradeCount();

                    user->updateLTP(symbol, price);
                    double signedQty = (side == "BUY") ? qty : -qty;
                    user->updatePosition(symbol, signedQty, price);
                }
            }

            freeReplyObject(reply);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("Redis stream error: {}", e.what());
        } catch (...) {
            SPDLOG_ERROR("Unknown Redis stream error");
        }
    }

    redisFree(redis);
    SPDLOG_INFO("🛑 Redis stream listener stopped.");
}

int main() {
    SPDLOG_INFO("🧠 Starting pnl-service...");

    try {
        ConfigLoader::loadEnv(".env");

        const std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        const int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));
        const std::string grpcPort = ConfigLoader::getEnv("PNL_GRPC_PORT", "50052");

        auto service = std::make_unique<PnLServiceImpl>();
        std::thread redisThread(streamTradeListener, service.get(), redisHost, redisPort);
        startHealthAndMetricsServer();  // ✅ new unified metrics + health

        std::string serverAddress = "0.0.0.0:" + grpcPort;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(service.get());

        auto server = builder.BuildAndStart();
        SPDLOG_INFO("✅ gRPC server listening on {}", serverAddress);

        std::signal(SIGINT, shutdownHandler);
        std::signal(SIGTERM, shutdownHandler);

        while (keepRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        server->Shutdown();
        redisThread.join();
        SPDLOG_INFO("👋 pnl-service shutdown complete.");

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("💥 Fatal error in main: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("💥 Unknown fatal error in main.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

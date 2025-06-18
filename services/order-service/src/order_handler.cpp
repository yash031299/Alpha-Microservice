#include "order_handler.hpp"
#include "config_loader.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>


using json = nlohmann::json;

OrderHandler::OrderHandler()
    : symbolClient(std::make_shared<SymbolClient>(
                            grpc::CreateChannel(ConfigLoader::get("SYMBOL_REGISTRY_GRPC", "localhost:50061"),
                            grpc::InsecureChannelCredentials())
                        )
                  ),
      redisClient(std::make_shared<RedisClient>(
                            ConfigLoader::get("REDIS_HOST", "127.0.0.1"),
                            std::stoi(ConfigLoader::get("REDIS_PORT", "6379"))
                        )
                 ) {}



bool OrderHandler::validateSymbol(const std::string& symbol) {
    auto meta = symbolClient->getSymbolMeta(symbol);
    return meta.has_value();
}

bool OrderHandler::checkMargin(const json& orderJson) {
    std::string userId = orderJson["userId"];
    std::string key = "QUEUE:MARGIN:CHECK:" + userId;
    redisClient->lpush(key, orderJson.dump());

    // Wait up to 2s for risk-service to reply on MARGIN:RESULT:<user>
    std::string resultKey = "MARGIN:RESULT:" + userId;
    int max_wait = 2000;
    int waited = 0;

    while (waited < max_wait) {
        auto res = redisClient->get(resultKey);
        if (!res.empty()) {
            redisClient->del(resultKey);
            return res == "ALLOW";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        waited += 100;
    }

    SPDLOG_WARN("Risk check timeout for user {}", userId);
    return false;
}

bool OrderHandler::submitOrder(const json& orderJson) {
    try {
        std::string symbol = orderJson["symbol"];

        // 1. Symbol validation
        if (!validateSymbol(symbol)) {
            SPDLOG_WARN("❌ Invalid symbol: {}", symbol);
            return false;
        }

        // 2. Risk check
        if (!checkMargin(orderJson)) {
            SPDLOG_WARN("❌ Risk check failed");
            return false;
        }

        // 3. Submit order to QUEUE:ORDER
        redisClient->lpush("QUEUE:ORDER", orderJson.dump());
        SPDLOG_INFO("✅ Order submitted to Redis QUEUE:ORDER");

        return true;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in submitOrder: {}", e.what());
        return false;
    }
}

#include "redis_connection.hpp"
#include "redis_retry.hpp"
#include "config_loader.hpp"
#include <ctime>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

RedisPublisher::RedisPublisher() {
    try {
        std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));
        ctx_ = safeRedisConnect(redisHost, redisPort);

        if (!ctx_ || ctx_->err) {
            SPDLOG_ERROR("❌ RedisPublisher connection failed: {}", ctx_ ? ctx_->errstr : "null");
        } else {
            SPDLOG_INFO("✅ RedisPublisher connected to {}:{}", redisHost, redisPort);
        }
    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ RedisPublisher init error: {}", ex.what());
        ctx_ = nullptr;
    }
}

void RedisPublisher::tradeMessage(const std::string& user_id,
                                  const std::string& symbol,
                                  double price,
                                  double quantity,
                                  const std::string& side,
                                  const std::string& type) {
    try {
        if (!ctx_) {
            SPDLOG_WARN("⚠️ Skipping Redis publish — no valid Redis connection.");
            return;
        }

        std::string payload = fmt::format(
            R"({{"user_id":"{}","symbol":"{}","price":{},"quantity":{},"side":"{}","type":"{}","timestamp":{}}})",
            user_id, symbol, price, quantity, side, type, std::time(nullptr)
        );

        redisReply* reply = safeRedisCommand(ctx_, "PUBLISH trades %s", payload.c_str());
        if (reply) {
            freeReplyObject(reply);
            SPDLOG_INFO("📤 Published trade: {}", payload);
        } else {
            SPDLOG_WARN("⚠️ Failed to publish trade for user: {}", user_id);
        }

    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ Redis publish error: {}", ex.what());
    }
}

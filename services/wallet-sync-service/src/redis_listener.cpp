#include "redis_listener.hpp"
#include "wallet_store.hpp"
#include "config_loader.hpp"
#include "redis_retry.hpp"

#include <spdlog/spdlog.h>
#include <hiredis/hiredis.h>
#include <rapidjson/document.h>
#include <thread>
#include <chrono>

extern WalletStore walletStore;

RedisListener::RedisListener() {
    redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
    redisPort = ConfigLoader::getEnvInt("REDIS_PORT", 6379);
    streamKey = ConfigLoader::getEnv("TRADE_STREAM_KEY", "STREAM:TRADE");
}

void RedisListener::start() {
    if (running) return;

    running = true;
    listenerThread = std::thread(&RedisListener::listen, this);
    SPDLOG_INFO("🚀 Redis listener started on stream '{}'", streamKey);
}

void RedisListener::stop() {
    running = false;
    if (listenerThread.joinable()) {
        listenerThread.join();
        SPDLOG_INFO("🛑 Redis listener thread stopped");
    }
}

void RedisListener::publishMarginCheck(const std::string& userId) {
    try {
        redisContext* redis = safeRedisConnect(redisHost, redisPort);
        if (!redis || redis->err) {
            SPDLOG_ERROR("❌ Redis error in publishMarginCheck: {}", redis ? redis->errstr : "null");
            return;
        }

        std::string queue = "QUEUE:MARGIN:CHECK:" + userId;
        redisCommand(redis, "RPUSH %s %s", queue.c_str(), userId.c_str());

        redisFree(redis);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in publishMarginCheck: {}", e.what());
    }
}

void RedisListener::listen() {
    std::string last_id = "$";  // `$` = latest entry, avoids replay on restart

    try {
        redisContext* redis = safeRedisConnect(redisHost, redisPort);
        if (!redis || redis->err) {
            SPDLOG_CRITICAL("❌ Redis connection failed: {}", redis ? redis->errstr : "null");
            return;
        }

        SPDLOG_INFO("📥 Listening to stream: {}", streamKey);

        while (running) {
            redisReply* reply = (redisReply*)redisCommand(redis,
                "XREAD COUNT 1 BLOCK 1000 STREAMS %s %s", streamKey.c_str(), last_id.c_str());

            if (!reply || reply->type != REDIS_REPLY_ARRAY) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }

            try {
                auto entries = reply->element[0]->element[1];
                for (size_t i = 0; i < entries->elements; ++i) {
                    auto entry = entries->element[i];
                    std::string entry_id = entry->element[0]->str;
                    last_id = entry_id;

                    std::string tradeJson = entry->element[1]->element[1]->str;

                    // Parse trade JSON
                    rapidjson::Document doc;
                    doc.Parse(tradeJson.c_str());

                    if (doc.HasParseError() || !doc.HasMember("user") || !doc.HasMember("pnl")) {
                        SPDLOG_WARN("🚫 Invalid trade JSON: {}", tradeJson);
                        continue;
                    }

                    std::string userId = doc["user"].GetString();
                    double pnl = doc["pnl"].GetDouble();

                    walletStore.applyTrade(userId, pnl);
                    publishMarginCheck(userId);
                }
            } catch (const std::exception& e) {
                SPDLOG_ERROR("Exception in stream processing: {}", e.what());
            }

            freeReplyObject(reply);
        }

        redisFree(redis);
    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("🔥 Fatal error in RedisListener: {}", e.what());
    }
}

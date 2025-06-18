#pragma once
#include "redis_retry.hpp"
#include "config_loader.hpp"
#include <optional>
#include <string>
#include <spdlog/spdlog.h>

// Fetch latest LTP for a given symbol from Redis
std::optional<double> getLatestLTP(const std::string& symbol, redisContext* ctx) {
    try {
        std::string key = "TICK:" + symbol;

        // First check the key's type to ensure it's a hash
        redisReply* typeReply = (redisReply*)redisCommand(ctx, "TYPE %s", key.c_str());
        if (!typeReply || typeReply->type != REDIS_REPLY_STATUS || std::string(typeReply->str) != "hash") {
            SPDLOG_WARN("🚫 Wrong type for Redis key {}. Expected 'hash', got '{}'", key, typeReply ? typeReply->str : "null");
            if (typeReply) freeReplyObject(typeReply);
            return std::nullopt;
        }
        freeReplyObject(typeReply);  // Free after successful type check

        // Now it's safe to HGET
        redisReply* reply = (redisReply*)redisCommand(ctx, "HGET %s price", key.c_str());
        if (!reply) {
            SPDLOG_WARN("🚫 No Redis reply for key {}", key);
            return std::nullopt;
        }

        if (reply->type != REDIS_REPLY_STRING) {
            SPDLOG_WARN("🚫 Unexpected reply type for HGET {}. Type = {}", key, reply->type);
            freeReplyObject(reply);
            return std::nullopt;
        }

        double ltp = std::stod(reply->str);
        SPDLOG_INFO("✅ getLatestLTP success. Key = {}, Price = {}", key, ltp);
        freeReplyObject(reply);
        return ltp;

    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ Exception in getLatestLTP: {}", ex.what());
        return std::nullopt;
    }
}


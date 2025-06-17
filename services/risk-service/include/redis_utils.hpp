#pragma once
#include "redis_retry.hpp"
#include "config_loader.hpp"
#include <optional>
#include <string>
#include <spdlog/spdlog.h>

// Fetch latest LTP for a given symbol from Redis
inline std::optional<double> getLatestLTP(const std::string& symbol, redisContext* ctx) {
    try {
        if (!ctx) return std::nullopt;

        std::string streamKey = ConfigLoader::getEnv("LTP_STREAM_KEY", "market_ticks");

        redisReply* reply = safeRedisCommand(ctx, "XREVRANGE %s + - COUNT 1", streamKey.c_str());
        if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements == 0) {
            if (reply) freeReplyObject(reply);
            return std::nullopt;
        }

        auto fields = reply->element[0]->element[1];
        double ltp = 0.0;

        for (size_t j = 0; j < fields->elements; j += 2) {
            std::string key = fields->element[j]->str;
            std::string val = fields->element[j + 1]->str;
            if (key == "ltp") {
                ltp = std::stod(val);
                break;
            }
        }

        freeReplyObject(reply);
        return ltp > 0.0 ? std::optional<double>{ltp} : std::nullopt;

    } catch (const std::exception& ex) {
        SPDLOG_ERROR("getLatestLTP failed: {}", ex.what());
        return std::nullopt;
    }
}

#pragma once
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

inline redisContext* safeRedisConnect(const std::string& host, int port, int retries = 3) {
    redisContext* ctx = nullptr;

    for (int attempt = 1; attempt <= retries; ++attempt) {
        ctx = redisConnect(host.c_str(), port);
        if (ctx && !ctx->err) {
            return ctx;
        }

        SPDLOG_WARN("Redis connect attempt {}/{} failed: {}", attempt, retries,
                    ctx ? ctx->errstr : "null");

        if (ctx) redisFree(ctx);
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt));
    }

    return nullptr;
}

inline redisReply* safeRedisCommand(redisContext* ctx, const char* format, ...) {
    if (!ctx) return nullptr;

    va_list args;
    va_start(args, format);
    redisReply* reply = (redisReply*)redisvCommand(ctx, format, args);
    va_end(args);

    return reply;
}

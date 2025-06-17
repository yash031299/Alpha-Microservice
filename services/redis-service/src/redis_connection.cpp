#include "redis_connection.hpp"
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

RedisConnection::RedisConnection() : ctx(nullptr) {
    const char* redisHost = std::getenv("REDIS_HOST");
    const char* redisPort = std::getenv("REDIS_PORT");

    host = redisHost ? redisHost : "127.0.0.1";
    port = redisPort ? std::stoi(redisPort) : 6379;

    connect();
}

void RedisConnection::connect() {
    try {
        ctx = redisConnect(host.c_str(), port);
        if (!ctx || ctx->err) {
            SPDLOG_ERROR("Redis connect failed to {}:{} - {}", host, port, ctx ? ctx->errstr : "null");
            if (ctx) redisFree(ctx);
            ctx = nullptr;
        } else {
            SPDLOG_INFO("✅ Connected to Redis at {}:{}", host, port);
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in RedisConnection::connect - {}", e.what());
        ctx = nullptr;
    }
}

RedisConnection::~RedisConnection() {
    if (ctx) redisFree(ctx);
}

redisContext* RedisConnection::get() {
    return ctx;
}

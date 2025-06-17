#include "redis_pool.hpp"
#include <spdlog/spdlog.h>

RedisPool::RedisPool(const std::string& host, int port, int poolSize)
    : maxSize_(poolSize) {

    for (int i = 0; i < maxSize_; ++i) {
        redisContext* ctx = redisConnect(host.c_str(), port);
        if (!ctx || ctx->err) {
            SPDLOG_ERROR("RedisPool failed to connect ({}): {}", i, ctx ? ctx->errstr : "null");
            if (ctx) redisFree(ctx);
        } else {
            pool_.push_back(ctx);
            SPDLOG_INFO("RedisPool connection {} established", i);
        }
    }

    if (pool_.empty()) {
        throw std::runtime_error("RedisPool failed to initialize any connections.");
    }
}

RedisPool::~RedisPool() {
    for (auto* ctx : pool_) {
        if (ctx) redisFree(ctx);
    }
}

redisContext* RedisPool::acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !pool_.empty(); });

    redisContext* ctx = pool_.back();
    pool_.pop_back();
    return ctx;
}

void RedisPool::release(redisContext* ctx) {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.push_back(ctx);
    cv_.notify_one();
}

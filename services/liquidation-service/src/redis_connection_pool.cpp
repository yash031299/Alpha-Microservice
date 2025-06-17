#include "redis_connection_pool.hpp"
#include <stdexcept>
#include <spdlog/spdlog.h>

RedisConnectionPool::RedisConnectionPool(const std::string& host, int port, int poolSize)
    : host(host), port(port) {
    for (int i = 0; i < poolSize; ++i) {
        redisContext* conn = redisConnect(host.c_str(), port);
        if (!conn || conn->err) {
            SPDLOG_CRITICAL("❌ Redis connection failed: {}", conn ? conn->errstr : "null");
            throw std::runtime_error("Redis connection failed");
        }
        pool.push(std::shared_ptr<redisContext>(conn, redisFree));
        if (redisCommand(conn, "PING") == nullptr) {
            SPDLOG_CRITICAL("❌ Redis connection failed ping");
            throw std::runtime_error("Redis ping failed");
        }

    }
    SPDLOG_INFO("🔌 Redis pool initialized with {} connections", poolSize);
}

std::shared_ptr<redisContext> RedisConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]() { return !pool.empty(); });

    auto conn = pool.front();
    if (!conn) {
        SPDLOG_ERROR("🚫 redisCommand called with null connection");
        return nullptr;

    }

    pool.pop();

    // Wrap again with outer shared_ptr but keep original ownership
    return std::shared_ptr<redisContext>(
        conn.get(),  // raw ptr
        [this, conn](redisContext* c) {
            std::lock_guard<std::mutex> lock(mutex);
            pool.push(conn);  // push back the original shared_ptr
            cv.notify_one();
        }
    );
}


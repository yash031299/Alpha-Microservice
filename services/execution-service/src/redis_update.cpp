#include "redis_update.hpp"
#include <sw/redis++/redis++.h>
#include <spdlog/spdlog.h>
#include <cstdlib>

RedisConnectionPool::RedisConnectionPool() : running_(true) {
    worker_ = std::thread(&RedisConnectionPool::workerLoop, this);
}

RedisConnectionPool::~RedisConnectionPool() {
    running_ = false;
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void RedisConnectionPool::enqueueUpdate(const std::string& order_id,
                                        const std::string& status,
                                        double filled_qty,
                                        double avg_price) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.emplace(order_id, status, filled_qty, avg_price);
    }
    cv_.notify_one();
}

void RedisConnectionPool::workerLoop() {
    std::string redis_url = std::getenv("REDIS_URL") ? std::getenv("REDIS_URL") : "tcp://localhost:6379";
    sw::redis::Redis redis(redis_url);

    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return !tasks_.empty() || !running_; });

        if (!running_) break;

        auto [order_id, status, filled_qty, avg_price] = tasks_.front();
        tasks_.pop();
        lock.unlock();

        try {
            std::string key = "ORDER:" + order_id;
            std::unordered_map<std::string, std::string> fields = {
                {"status", status},
                {"executed_qty", std::to_string(filled_qty)},
                {"avg_price", std::to_string(avg_price)}
            };
            redis.hmset(key, fields.begin(), fields.end());

            SPDLOG_INFO("✅ Redis updated for {}", key);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("❌ Failed Redis update for {}: {}", order_id, e.what());
        }
    }
}

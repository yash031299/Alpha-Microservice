#include "redis_publisher.hpp"
#include <spdlog/spdlog.h>
#include <iostream>

RedisPublisher::RedisPublisher(const std::string& host, int port, const std::string& streamKey)
    : redisHost_(host), redisPort_(port), streamKey_(streamKey), stop_(false) {
    worker_ = std::thread(&RedisPublisher::processQueue, this);
}

RedisPublisher::~RedisPublisher() {
    stop_ = true;
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void RedisPublisher::enqueue(const std::string& jsonPayload) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(jsonPayload);
    }
    cv_.notify_one();
}

void RedisPublisher::processQueue() {
    while (!stop_) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty() || stop_; });

        while (!queue_.empty()) {
            std::string payload = queue_.front();
            queue_.pop();
            lock.unlock();

            redisContext* conn = createConnection();
            if (!conn) {
                SPDLOG_ERROR("Redis connection failed while publishing order");
                break;
            }

            redisReply* reply = (redisReply*)redisCommand(conn,
                "XADD %s * order_json %s", streamKey_.c_str(), payload.c_str());

            if (reply == nullptr) {
                SPDLOG_ERROR("Redis command failed");
            } else {
                freeReplyObject(reply);
                SPDLOG_INFO("📤 Pushed failed order to Redis stream: {}", streamKey_);
            }

            redisFree(conn);
            lock.lock();
        }
    }
}

redisContext* RedisPublisher::createConnection() {
    redisContext* conn = redisConnect(redisHost_.c_str(), redisPort_);
    if (conn && conn->err) {
        SPDLOG_ERROR("Redis error: {}", conn->errstr);
        redisFree(conn);
        return nullptr;
    }
    return conn;
}

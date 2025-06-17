#include "redis_publisher.hpp"
#include <spdlog/spdlog.h>

RedisPublisher::RedisPublisher(const std::string& host, int port)
    : context_(nullptr), host_(host), port_(port) {
    reconnect();
}

RedisPublisher::~RedisPublisher() {
    if (context_) redisFree(context_);
}

bool RedisPublisher::reconnect() {
    if (context_) redisFree(context_);
    context_ = redisConnect(host_.c_str(), port_);
    if (!context_ || context_->err) {
        SPDLOG_ERROR("❌ Redis connection failed: {}", context_ ? context_->errstr : "null");
        context_ = nullptr;
        return false;
    }
    SPDLOG_INFO("✅ Connected to Redis at {}:{}", host_, port_);
    return true;
}

bool RedisPublisher::publishSnapshot(const std::string& symbol, const std::string& snapshotJson) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) {
        SPDLOG_WARN("⚠️ No Redis connection. Attempting reconnect...");
        if (!reconnect()) return false;
    }

    std::string streamKey = "STREAM:SNAPSHOT:" + symbol;

    redisReply* reply = static_cast<redisReply*>(
        redisCommand(context_, "XADD %s * data %s", streamKey.c_str(), snapshotJson.c_str())
    );

    if (!reply) {
        SPDLOG_ERROR("❌ Failed to publish snapshot to Redis: {}", streamKey);
        reconnect();  // attempt reconnect on failure
        return false;
    }

    freeReplyObject(reply);
    SPDLOG_INFO("📤 Snapshot published → Redis [{}]", streamKey);
    return true;
}

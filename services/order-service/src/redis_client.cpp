#include "redis_client.hpp"
#include <spdlog/spdlog.h>

RedisClient::RedisClient(const std::string& host, int port) {
    context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err) {
        SPDLOG_ERROR("❌ Failed to connect to Redis: {}", context ? context->errstr : "unknown");
        throw std::runtime_error("Redis connection failed");
    }
    SPDLOG_INFO("✅ Connected to Redis at {}:{}", host, port);
}

RedisClient::~RedisClient() {
    if (context) {
        redisFree(context);
    }
}

void RedisClient::lpush(const std::string& key, const std::string& value) {
    redisCommand(context, "LPUSH %s %s", key.c_str(), value.c_str());
}

std::string RedisClient::get(const std::string& key) {
    redisReply* reply = (redisReply*)redisCommand(context, "GET %s", key.c_str());
    std::string result;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        result = reply->str;
    }
    freeReplyObject(reply);
    return result;
}

void RedisClient::del(const std::string& key) {
    redisCommand(context, "DEL %s", key.c_str());
}

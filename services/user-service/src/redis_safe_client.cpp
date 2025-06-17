#include "redis_safe_client.hpp"
#include <spdlog/spdlog.h>
#include <cstdarg>

RedisSafeClient::RedisSafeClient(const std::string& host, int port)
    : host_(host), port_(port), ctx_(nullptr) {
    connect();
}

RedisSafeClient::~RedisSafeClient() {
    if (ctx_) redisFree(ctx_);
}

bool RedisSafeClient::connect() {
    try {
        if (ctx_) redisFree(ctx_);
        ctx_ = redisConnect(host_.c_str(), port_);
        if (!ctx_ || ctx_->err) {
            SPDLOG_ERROR("Redis connect failed: {}", ctx_ ? ctx_->errstr : "nullptr");
            return false;
        }
        SPDLOG_INFO("Connected to Redis at {}:{}", host_, port_);
        return true;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception during Redis connect: {}", e.what());
        return false;
    } catch (...) {
        SPDLOG_ERROR("Unknown error during Redis connect");
        return false;
    }
}

bool RedisSafeClient::isConnected() const {
    return ctx_ && ctx_->err == 0;
}

redisReply* RedisSafeClient::command(const char* format, ...) {
    try {
        if (!isConnected()) {
            SPDLOG_WARN("Redis not connected, attempting reconnect with retry...");
            if (!connectWithRetry()) {
                SPDLOG_ERROR("Redis reconnection failed. Command aborted.");
                return nullptr;
            }
        }

        va_list args;
        va_start(args, format);
        redisReply* reply = (redisReply*)redisvCommand(ctx_, format, args);
        va_end(args);

        if (!reply) {
            SPDLOG_ERROR("Redis command failed");
            return nullptr;
        }

        return reply;
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception during Redis command: {}", e.what());
        return nullptr;
    } catch (...) {
        SPDLOG_ERROR("Unknown exception during Redis command");
        return nullptr;
    }
}

bool RedisSafeClient::connectWithRetry(int maxRetries, int baseDelayMs) {
    int attempt = 0;
    while (attempt < maxRetries) {
        if (connect()) {
            return true;
        }
        int delay = baseDelayMs * (1 << attempt);  // exponential backoff
        SPDLOG_WARN("Redis connect failed, retrying in {} ms (attempt {}/{})", delay, attempt + 1, maxRetries);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        attempt++;
    }
    SPDLOG_ERROR("Redis connect failed after {} attempts", maxRetries);
    return false;
}

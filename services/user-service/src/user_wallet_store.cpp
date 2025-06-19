#include "user_wallet_store.hpp"
#include <spdlog/spdlog.h>

UserWalletStore::UserWalletStore(std::shared_ptr<RedisSafeClient> redis)
    : redis_(std::move(redis)) {}


    
void UserWalletStore::initializeFromRedis() {
    try {
        redisReply* reply = redis_->command("HGETALL user_wallets");
        if (!reply || reply->type != REDIS_REPLY_ARRAY) {
            SPDLOG_ERROR("Failed to load user_wallets from Redis");
            if (reply) freeReplyObject(reply);
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        walletMap_.clear();
        for (size_t i = 0; i < reply->elements; i += 2) {
            std::string userId  = reply->element[i]->str;
            std::string balance = reply->element[i + 1]->str;
            walletMap_[userId] = std::stod(balance);
        }

        SPDLOG_INFO("Loaded {} wallet balances from Redis", walletMap_.size());
        freeReplyObject(reply);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in initializeFromRedis: {}", e.what());
    }
}

void UserWalletStore::applyDelta(const std::string& userId, double delta) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        walletMap_[userId] += delta;
        SPDLOG_INFO("Updated wallet {} by delta {:.2f}. New balance: {:.2f}",
                    userId, delta, walletMap_[userId]);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in applyDelta for user {}: {}", userId, e.what());
    }
}

void UserWalletStore::flushToRedis() {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [userId, balance] : walletMap_) {
            redisReply* reply = redis_->command("HSET user_wallets %s %f", userId.c_str(), balance);
            if (reply) freeReplyObject(reply);
        }
        SPDLOG_INFO("Flushed {} balances to Redis", walletMap_.size());
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in flushToRedis: {}", e.what());
    }
}

double UserWalletStore::getBalance(const std::string& userId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = walletMap_.find(userId);
    return (it != walletMap_.end()) ? it->second : 0.0;
}


std::shared_ptr<RedisSafeClient> UserWalletStore::getRedis() const {
    return redis_;
}


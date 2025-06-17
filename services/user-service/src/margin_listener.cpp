#include "margin_listener.hpp"
#include <spdlog/spdlog.h>
#include <hiredis/hiredis.h>
#include <sstream>
#include <thread>
#include <chrono>

MarginEventListener::MarginEventListener(std::shared_ptr<RedisSafeClient> redis,
                                         std::shared_ptr<UserWalletStore> walletStore)
    : redis_(std::move(redis)), walletStore_(std::move(walletStore)) {}

void MarginEventListener::start() {
    running_ = true;
    thread_ = std::thread([this]() {
        redisContext* ctx = nullptr;

        while (running_) {
            try {
                if (!ctx) {
                    ctx = redisConnect("127.0.0.1", 6379);
                    if (!ctx || ctx->err) {
                        SPDLOG_ERROR("MarginListener Redis connect error: {}", ctx ? ctx->errstr : "null");
                        if (ctx) redisFree(ctx);
                        ctx = nullptr;
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        continue;
                    }
                    redisReply* subReply = (redisReply*)redisCommand(ctx, "SUBSCRIBE MARGIN_UPDATES");
                    if (!subReply) {
                        SPDLOG_ERROR("Failed to subscribe MARGIN_UPDATES");
                        redisFree(ctx);
                        ctx = nullptr;
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        continue;
                    }
                    freeReplyObject(subReply);
                    SPDLOG_INFO("Subscribed to MARGIN_UPDATES");
                }

                redisReply* reply = nullptr;
                if (redisGetReply(ctx, (void**)&reply) == REDIS_OK && reply) {
                    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                        std::string message = reply->element[2]->str;
                        size_t sep = message.find(':');
                        if (sep != std::string::npos) {
                            std::string userId = message.substr(0, sep);
                            double delta = std::stod(message.substr(sep + 1));
                            walletStore_->applyDelta(userId, delta);
                        }
                    }
                    freeReplyObject(reply);
                } else {
                    SPDLOG_WARN("Redis reply failed, reconnecting...");
                    if (ctx) {
                        redisFree(ctx);
                        ctx = nullptr;
                    }
                }
            } catch (const std::exception& e) {
                SPDLOG_ERROR("Exception in MarginListener loop: {}", e.what());
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } catch (...) {
                SPDLOG_ERROR("Unknown exception in MarginListener loop");
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if (ctx) {
            redisFree(ctx);
            ctx = nullptr;
        }
        SPDLOG_INFO("MarginListener thread exiting");
    });
}

void MarginEventListener::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

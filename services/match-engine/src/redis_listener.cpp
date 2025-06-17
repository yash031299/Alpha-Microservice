#include "redis_listener.hpp"
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include "order_parser.hpp"
#include "trade_publisher.hpp"


RedisListener::RedisListener(const std::string& host, int port, const std::string& streamKey, Matcher& matcher)
    : host_(host), port_(port), streamKey_(streamKey), matcher_(matcher), lastId_("0-0"), running_(false) {}

RedisListener::~RedisListener() {
    stop();
}

void RedisListener::start() {
    running_ = true;
    worker_ = std::thread(&RedisListener::listen, this);
}

void RedisListener::stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
}

void RedisListener::listen() {
    SPDLOG_INFO("📡 Redis listener started on stream: {}", streamKey_);

    redisContext* ctx = redisConnect(host_.c_str(), port_);
    if (!ctx || ctx->err) {
        SPDLOG_ERROR("❌ Failed to connect to Redis: {}", ctx ? ctx->errstr : "null context");
        return;
    }

    while (running_) {
        try {
            std::string cmd = "XREAD BLOCK 5000 STREAMS " + streamKey_ + " " + lastId_;
            redisReply* reply = (redisReply*)redisCommand(ctx, cmd.c_str());

            if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements == 0) {
                freeReplyObject(reply);
                continue;
            }

            redisReply* stream = reply->element[0];
            if (stream->elements < 2) {
                freeReplyObject(reply);
                continue;
            }

            redisReply* messages = stream->element[1];
            for (size_t i = 0; i < messages->elements; ++i) {
                redisReply* entry = messages->element[i];
                std::string entryId = entry->element[0]->str;
                lastId_ = entryId;

                redisReply* fields = entry->element[1];
                for (size_t j = 0; j + 1 < fields->elements; j += 2) {
                    std::string key = fields->element[j]->str;
                    std::string val = fields->element[j + 1]->str;

                    if (key == "order") {
                        try {
                            Order order = OrderParser::parse(val);
                            auto trades = matcher_.submit(order);
                            TradePublisher().publish(trades);  // TODO: replace with shared publisher
                        } catch (const std::exception& e) {
                            SPDLOG_ERROR("⚠️ Failed to parse order: {}", e.what());
                        }
                    }

                }
            }

            freeReplyObject(reply);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("💥 Redis listen loop crashed: {}", e.what());
        } catch (...) {
            SPDLOG_ERROR("💥 Unknown error in Redis listener");
        }
    }

    redisFree(ctx);
    SPDLOG_INFO("📴 Redis listener stopped");
}

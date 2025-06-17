#include "redis_stream_listener.hpp"
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

void RedisStreamListener::loadEnv() {
    std::ifstream envFile(".env");
    std::string line;
    while (std::getline(envFile, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        setenv(key.c_str(), value.c_str(), 1);
    }

    redis_host_ = std::getenv("REDIS_HOST") ? std::getenv("REDIS_HOST") : "127.0.0.1";
    redis_port_ = std::getenv("REDIS_PORT") ? std::stoi(std::getenv("REDIS_PORT")) : 6379;
    stream_key_ = std::getenv("TICK_STREAM_KEY") ? std::getenv("TICK_STREAM_KEY") : "market_ticks";
    pub_channel_ = std::getenv("REDIS_PUB_CHANNEL") ? std::getenv("REDIS_PUB_CHANNEL") : "TICKS:PUB";
}

RedisStreamListener::RedisStreamListener() {
    loadEnv();
}

void RedisStreamListener::run() {
    listen();
}

void RedisStreamListener::listen() {
    try {
        redisContext* ctx = redisConnect(redis_host_.c_str(), redis_port_);
        if (!ctx || ctx->err) {
            SPDLOG_ERROR("❌ Redis connection error: {}", ctx ? ctx->errstr : "unknown error");
            return;
        }

        std::string last_id = "$";

        while (true) {
            redisReply* reply = (redisReply*)redisCommand(ctx, "XREAD BLOCK 5000 STREAMS %s %s", stream_key_.c_str(), last_id.c_str());

            if (!reply) {
                SPDLOG_WARN("⚠️ Null reply from Redis");
                continue;
            }

            if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
                for (size_t i = 0; i < reply->elements; ++i) {
                    redisReply* stream_data = reply->element[i];
                    if (stream_data->type != REDIS_REPLY_ARRAY || stream_data->elements < 2) continue;

                    redisReply* entries = stream_data->element[1];
                    for (size_t j = 0; j < entries->elements; ++j) {
                        redisReply* entry = entries->element[j];
                        if (entry->type == REDIS_REPLY_ARRAY && entry->elements == 2) {
                            last_id = entry->element[0]->str;
                            redisReply* fields = entry->element[1];

                            std::string pubPayload;
                            for (size_t k = 0; k + 1 < fields->elements; k += 2) {
                                if (fields->element[k]->str && fields->element[k + 1]->str) {
                                    std::string key = fields->element[k]->str;
                                    std::string value = fields->element[k + 1]->str;
                                    pubPayload += "\"" + key + "\":\"" + value + "\",";
                                }
                            }

                            if (!pubPayload.empty()) {
                                pubPayload.pop_back(); // remove trailing comma
                                pubPayload = "{" + pubPayload + "}";

                                redisReply* pubReply = (redisReply*)redisCommand(ctx, "PUBLISH %s %s", pub_channel_.c_str(), pubPayload.c_str());
                                if (pubReply) {
                                    SPDLOG_INFO("📤 Published tick to {}: {}", pub_channel_, pubPayload);
                                    freeReplyObject(pubReply);
                                } else {
                                    SPDLOG_WARN("⚠️ Failed to publish tick");
                                }
                            }
                        }
                    }
                }
            }
            freeReplyObject(reply);
        }

        redisFree(ctx);
    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ Exception in stream loop: {}", ex.what());
    } catch (...) {
        SPDLOG_ERROR("❌ Unknown exception in stream loop");
    }
}

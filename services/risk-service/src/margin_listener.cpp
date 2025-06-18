#include "margin_listener.hpp"
#include "rms/rms_engine.hpp"
#include "redis_retry.hpp"
#include "config_loader.hpp"
#include "thread_safe_map.hpp"
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>
#include <chrono>
#include <memory>
#include <thread>
#include <optional>

// Correct declaration of shared global map from main.cpp
extern ThreadSafeMap<std::string, std::shared_ptr<rms::RMSEngine>> rmsMap;

void MarginListener::start() {
    running = true;
    listenerThread = std::thread([this] { 
        this->listen(); 
    });
}

void MarginListener::stop() {
    running = false;
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
}

void MarginListener::listen() {
    SPDLOG_INFO("📥 Starting MarginListener on QUEUE:MARGIN:CHECK:*");
    ConfigLoader::loadEnv(".env");


    redisContext* redis = nullptr;

    try {
        std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));
        SPDLOG_INFO("🔌 Connecting to Redis at {}:{}", redisHost, redisPort);

        redis = safeRedisConnect(redisHost, redisPort);
        if (!redis || redis->err) {
            SPDLOG_ERROR("❌ Redis connection failed in MarginListener: {}", redis ? redis->errstr : "null");
            if (redis) redisFree(redis);
            return;
        }

        SPDLOG_INFO("✅ Connected to Redis");

        while (running) {
            try {
                std::string queueList = "QUEUE:MARGIN:CHECK:user-001 QUEUE:MARGIN:CHECK:user-002 QUEUE:MARGIN:CHECK:user-003";
                redisReply* reply = (redisReply*)redisCommand(redis, "BLPOP %s 0", queueList.c_str());

                if (!reply) {
                    SPDLOG_ERROR("❌ Redis reply is null.");
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }

                if (reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
                    SPDLOG_ERROR("❌ Redis reply not array or has <2 elements.");
                    freeReplyObject(reply);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }

                if (!reply->element[1] || reply->element[1]->type != REDIS_REPLY_STRING || !reply->element[1]->str) {
                    SPDLOG_ERROR("❌ Redis reply element[1] invalid or not string.");
                    freeReplyObject(reply);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }

                std::string userId = reply->element[1]->str;
                SPDLOG_INFO("🧠 Received margin check trigger for user: {}", userId);

                try {
                    auto engineOpt = rmsMap.get(userId);
                    if (engineOpt.has_value()) {
                        auto& rms = engineOpt.value();
                        rms->syncMargin();
                        rms->evaluateRisk();
                    } else {
                        SPDLOG_WARN("⚠️ RMS not initialized for user: {}", userId);
                    }
                } catch (const std::exception& ex) {
                    SPDLOG_ERROR("🔥 MarginListener failed for {}: {}", userId, ex.what());
                }

                freeReplyObject(reply);

            } catch (const std::exception& ex) {
                SPDLOG_CRITICAL("🔥 Exception inside margin loop: {}", ex.what());
            } catch (...) {
                SPDLOG_CRITICAL("🔥 Unknown exception in margin loop");
            }
        }

    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("❌ MarginListener thread crashed: {}", ex.what());
    }

    if (redis) redisFree(redis);
}

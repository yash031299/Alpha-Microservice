#include "order_router.hpp"
#include "grpc_client.hpp"
#include "include/utils/logger.h"
#include <rapidjson/document.h>
#include <cstdlib>

OrderRouterService::OrderRouterService(std::shared_ptr<RedisConnectionPool> redis, std::shared_ptr<userManager> userMng)
    : redisPool(redis), userMng(userMng), running(false) {
    
    redis = redisConnect("127.0.0.1", 6379);
    if (!redis || redis->err) {
        SPD_CRITICAL("Redis connection error: {}", redis ? redis->errstr : "unknown");
        exit(1);
    }

    redisQueueName = getEnvOrDefault("ORDER_QUEUE", "QUEUE:ORDER");
    redisTimeout = std::stoi(getEnvOrDefault("REDIS_TIMEOUT", "5"));
}

OrderRouterService::~OrderRouterService() {
    stop();
    if (redis) redisFree(redis);
}

void OrderRouterService::start() {
    running = true;
    listenerThread = std::thread(&OrderRouterService::listenLoop, this);
}

void OrderRouterService::stop() {
    running = false;
    if (listenerThread.joinable()) listenerThread.join();
}

std::string OrderRouterService::getEnvOrDefault(const std::string& key, const std::string& defaultVal) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultVal;
}

void OrderRouterService::listenLoop() {
    SPD_INFO("🧭 OrderRouterService: Listening on {}", redisQueueName);

    while (running) {
        try {
            redisReply* reply = (redisReply*)redisCommand(redis, "BLPOP %s %d", redisQueueName.c_str(), redisTimeout);
            if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
                if (reply) freeReplyObject(reply);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::string jsonOrder(reply->element[1]->str);
            processOrder(jsonOrder);
            freeReplyObject(reply);
        } catch (const std::exception& e) {
            SPD_ERROR("Redis listener exception: {}", e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (...) {
            SPD_CRITICAL("Unknown error in listenLoop");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void OrderRouterService::processOrder(const std::string& jsonString) {
    rapidjson::Document doc;
    doc.Parse(jsonString.c_str());

    if (doc.HasParseError() || !doc.HasMember("orderType") || !doc.HasMember("symbol")) {
        SPD_ERROR("❌ Invalid order JSON: {}", jsonString);
        return;
    }

    const std::string orderType = doc["orderType"].GetString();
    const std::string symbol = doc["symbol"].GetString();

    if (orderType == "MARKET" || orderType == "SL") {
        try {
            std::string host = getEnvOrDefault("EXECUTION_HOST", "localhost:50051");
            ExecutionGRPCClient execClient(host);
            if (!execClient.sendOrderFromJson(jsonString))
                SPD_ERROR("🚨 execution-service gRPC failed");
        } catch (const std::exception& e) {
            SPD_ERROR("gRPC execution failed: {}", e.what());
        }
    } else if (orderType == "LIMIT") {
        std::string matchQueue = "QUEUE:MATCH:" + symbol;
        redisCommand(redis, "RPUSH %s %s", matchQueue.c_str(), jsonString.c_str());
        SPD_INFO("📤 Forwarded LIMIT order to {}", matchQueue);
    } else {
        SPD_WARN("⚠️ Unknown orderType '{}'", orderType);
    }
}

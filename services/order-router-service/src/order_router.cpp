#include "order_router.hpp"
#include "grpc_client.hpp"
#include "match_engine_client.hpp"
#include "rms_client.hpp"
#include "match.pb.h"
#include <spdlog/spdlog.h>
#include <rapidjson/document.h>
#include <cstdlib>
#include <chrono>
#include <thread>

OrderRouterService::OrderRouterService(const std::string& redis_host, int redis_port,
                                       std::shared_ptr<MatchEngineClient> matchClient,
                                       std::shared_ptr<RMSClient> rmsClient)
    : matchClient_(matchClient), rmsClient_(rmsClient), running_(false) {
    redis_ = redisConnect(redis_host.c_str(), redis_port);
    if (!redis_ || redis_->err) {
        SPDLOG_CRITICAL("Redis connection failed: {}", redis_ ? redis_->errstr : "unknown");
        exit(1);
    }

    redisQueueName_ = getEnvOrDefault("ORDER_QUEUE", "QUEUE:ORDER");
    redisTimeout_ = std::stoi(getEnvOrDefault("REDIS_TIMEOUT", "5"));
}

OrderRouterService::~OrderRouterService() {
    stop();
    if (redis_) redisFree(redis_);
}

void OrderRouterService::start() {
    running_ = true;
    listenerThread_ = std::thread(&OrderRouterService::listenLoop, this);
}

void OrderRouterService::stop() {
    running_ = false;
    if (listenerThread_.joinable()) listenerThread_.join();
}

std::string OrderRouterService::getEnvOrDefault(const std::string& key, const std::string& defaultVal) {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : defaultVal;
}

void OrderRouterService::listenLoop() {
    SPDLOG_INFO("🧭 OrderRouterService listening on queue: {}", redisQueueName_);

    while (running_) {
        try {
            redisReply* reply = (redisReply*)redisCommand(redis_, "BLPOP %s %d", redisQueueName_.c_str(), redisTimeout_);
            if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
                if (reply) freeReplyObject(reply);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::string jsonOrder(reply->element[1]->str);
            processOrder(jsonOrder);
            freeReplyObject(reply);
        } catch (const std::exception& ex) {
            SPDLOG_ERROR("❌ Error in listenLoop: {}", ex.what());
        } catch (...) {
            SPDLOG_CRITICAL("❌ Unknown error in listenLoop");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void OrderRouterService::processOrder(const std::string& jsonString) {
    rapidjson::Document doc;
    doc.Parse(jsonString.c_str());

    if (doc.HasParseError() || !doc.HasMember("orderType") || !doc.HasMember("symbol")) {
        SPDLOG_ERROR("❌ Invalid order JSON: {}", jsonString);
        return;
    }

    const std::string orderType = doc["orderType"].GetString();
    const std::string symbol = doc["symbol"].GetString();

    if (orderType == "MARKET" || orderType == "SL") {
        try {
            std::string execHost = getEnvOrDefault("EXECUTION_HOST", "localhost:50051");
            ExecutionGRPCClient execClient(execHost);
            // Step 0: Extract userId
            const std::string userId = doc["userId"].GetString();

            // Step 1: Check margin via RMS
            bool ok = false;
            double equity = rmsClient_->getEquity(userId, ok);
            if (!ok || equity < 1.0) {
                SPDLOG_WARN("🚫 User {} has insufficient equity ({:.2f})", userId, equity);
                return;
            }

            // Step 2: Proceed to execution
            if (!execClient.sendOrderFromJson(jsonString)) {
                SPDLOG_ERROR("🚨 Execution gRPC failed");
            }

        } catch (const std::exception& ex) {
            SPDLOG_ERROR("⚠️ gRPC execution failed: {}", ex.what());
        }
    } else if (orderType == "LIMIT") {
        try {
            // Step 1: Check margin via RMS
            const std::string userId = doc["userId"].GetString();
            bool ok = false;
            double equity = rmsClient_->getEquity(userId, ok);
            if (!ok || equity < 1.0) {
                SPDLOG_WARN("🚫 User {} has insufficient equity ({:.2f})", userId, equity);
                return;
            }

            // Step 2: Build match::OrderRequest
            match::OrderRequest matchReq;
            if (doc.HasMember("orderId"))        matchReq.set_orderid(doc["orderId"].GetString());
            if (doc.HasMember("userId"))         matchReq.set_userid(doc["userId"].GetString());
            if (doc.HasMember("symbol"))         matchReq.set_symbol(doc["symbol"].GetString());
            if (doc.HasMember("price"))          matchReq.set_price(doc["price"].GetDouble());
            if (doc.HasMember("quantity"))       matchReq.set_quantity(doc["quantity"].GetDouble());
            if (doc.HasMember("direction"))      matchReq.set_direction(doc["direction"].GetString());
            if (doc.HasMember("ordertype"))      matchReq.set_ordertype(doc["ordertype"].GetString());
            if (doc.HasMember("tif"))            matchReq.set_tif(doc["tif"].GetString());
            if (doc.HasMember("trigger_price"))  matchReq.set_trigger_price(doc["trigger_price"].GetDouble());

            // Step 3: Submit to match engine
            match::TradeResponse matchResp;
            if (!matchClient_->submitOrder(matchReq, matchResp)) {
                SPDLOG_ERROR("❌ Failed to submit LIMIT order to match-engine");
                return;
            }

            SPDLOG_INFO("📤 LIMIT order sent to match-engine, received {} trades", matchResp.trades_size());

        } catch (const std::exception& ex) {
            SPDLOG_ERROR("❌ Exception processing LIMIT order: {}", ex.what());
        }
    }

}

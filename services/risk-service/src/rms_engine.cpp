#include "rms/rms_engine.hpp"
#include "redis_retry.hpp"
#include "config_loader.hpp"
#include "liquidation_manager.hpp"
#include "rms/metrics.hpp"

#include <spdlog/spdlog.h>
#include <mutex>
#include <cmath>
#include <optional>
namespace rms {
RMSEngine::RMSEngine(const UserState& state)
    : state_(state), lastLTP_(0.0), marginUsed_(0.0), unrealizedPnL_(0.0) {}

void RMSEngine::onPriceUpdate(double ltp) {
    std::lock_guard<std::mutex> guard(lock_);
    lastLTP_ = ltp;

    try {
        std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));

        redisContext* ctx = safeRedisConnect(redisHost, redisPort);
        if (!ctx || ctx->err) {
            SPDLOG_ERROR("Redis state fetch failed for {}: {}", state_.userId, ctx ? ctx->errstr : "null ctx");
            if (ctx) redisFree(ctx);
            return;
        }

        redisReply* reply = safeRedisCommand(ctx,
            "HMGET rms:%s:state entryPrice positionSize side leverage walletBalance symbol",
            state_.userId.c_str());

        if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements >= 6) {
            state_.entryPrice     = std::stod(reply->element[0]->str);
            state_.positionSize   = std::stod(reply->element[1]->str);
            state_.side           = static_cast<rms::PositionSide>(std::stoi(reply->element[2]->str));
            state_.leverage       = std::stod(reply->element[3]->str);
            state_.walletBalance  = std::stod(reply->element[4]->str);
            state_.symbol         = reply->element[5]->str;
        } else {
            SPDLOG_WARN("Incomplete or missing user state for {} in Redis", state_.userId);
        }

        if (reply) freeReplyObject(reply);
        redisFree(ctx);

        calculatePnL();
        evaluateRisk();

    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ onPriceUpdate failed for {}: {}", state_.userId, ex.what());
    }
}

void RMSEngine::calculatePnL() {
    try {
        double notional = state_.entryPrice * state_.positionSize;
        double margin = notional / state_.leverage;

        unrealizedPnL_ = state_.positionSize * (lastLTP_ - state_.entryPrice) * static_cast<int>(state_.side);
        marginUsed_ = margin;

        SPDLOG_INFO("[{}] 🔄 PnL: {:.2f}, Margin: {:.2f}, Wallet: {:.2f}", state_.userId, unrealizedPnL_, marginUsed_, state_.walletBalance);
    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ calculatePnL failed: {}", ex.what());
    }
}

void RMSEngine::evaluateRisk() {
    try {
        double maintMarginRate = 0.05;
        double equity = state_.walletBalance + unrealizedPnL_;
        double threshold = marginUsed_ * maintMarginRate;

        SPDLOG_INFO("[{}] 📊 Equity: {:.2f}, Threshold: {:.2f}", state_.userId, equity, threshold);

        RMSMetrics::get().reportEquity(state_.userId, equity);

        if (equity < threshold) {
            checkLiquidation();
        } else {
            SPDLOG_INFO("[{}] ✅ Passed RMS check", state_.userId);
        }
    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ evaluateRisk failed: {}", ex.what());
    }
}

void RMSEngine::checkLiquidation() {
    try {
        SPDLOG_WARN("[{}] ⚠️ Triggering liquidation!", state_.userId);
        LiquidationManager manager;

        manager.liquidate(state_.userId,
                          state_.symbol,
                          state_.positionSize,
                          static_cast<int>(state_.side),
                          lastLTP_);

        RMSMetrics::get().incrementLiquidation();

    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ checkLiquidation failed for {}: {}", state_.userId, ex.what());
    }
}

void RMSEngine::syncMargin() {
    try {

        if (syncingMargin_.exchange(true)) return;  // already syncing

        std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));

        redisContext* ctx = safeRedisConnect(redisHost, redisPort);
        if (!ctx || ctx->err) {
            SPDLOG_ERROR("Redis sync failed for {}: {}", state_.userId, ctx ? ctx->errstr : "null");
            if (ctx) redisFree(ctx);
            return;
        }

        std::string key = "rms:" + state_.userId;
        redisReply* reply = safeRedisCommand(ctx,
            "HMSET %s ltp %f pnl %f margin %f equity %f",
            key.c_str(), lastLTP_, unrealizedPnL_, marginUsed_, (unrealizedPnL_ + state_.walletBalance)
        );

        if (reply) freeReplyObject(reply);
        redisFree(ctx);

        RMSMetrics::get().reportEquity(state_.userId, state_.walletBalance + unrealizedPnL_);
        SPDLOG_INFO("[{}] 🔄 Synced RMS to Redis", state_.userId);
        
        syncingMargin_ = false;

    } catch (const std::exception& ex) {
        SPDLOG_ERROR("❌ syncMargin failed for {}: {}", state_.userId, ex.what());
    }
}
}
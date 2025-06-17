#include "wallet_store.hpp"
#include "config_loader.hpp"
#include "redis_retry.hpp"

#include <spdlog/spdlog.h>
#include <hiredis/hiredis.h>

void WalletStore::applyTrade(const std::string& userId, double pnl) {
    std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
    int redisPort = ConfigLoader::getEnvInt("REDIS_PORT", 6379);
    int maxRetries = 3;
    int backoffMs = 100;

    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        try {
            redisContext* ctx = safeRedisConnect(redisHost, redisPort);
            if (!ctx || ctx->err) {
                SPDLOG_WARN("Retry {}/{} — Redis connect failed: {}", attempt, maxRetries,
                            ctx ? ctx->errstr : "null");
                if (ctx) redisFree(ctx);
                std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
                backoffMs *= 2;
                continue;
            }

            redisReply* reply = safeRedisCommand(ctx,
                "HINCRBYFLOAT wallet:%s balance %f", userId.c_str(), pnl);

            if (reply) {
                SPDLOG_INFO("💰 Wallet updated — User: {} | ΔPnL: {:.4f}", userId, pnl);
                freeReplyObject(reply);
                redisFree(ctx);
                return;
            } else {
                SPDLOG_WARN("Retry {}/{} — Redis command failed", attempt, maxRetries);
                redisFree(ctx);
                std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
                backoffMs *= 2;
            }
        } catch (const std::exception& e) {
            SPDLOG_ERROR("Retry {}/{} — Exception in wallet update: {}", attempt, maxRetries, e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
            backoffMs *= 2;
        }
    }

    SPDLOG_ERROR("❌ All {} attempts failed to update wallet for {}", maxRetries, userId);
}


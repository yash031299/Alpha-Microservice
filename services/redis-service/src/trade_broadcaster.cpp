#include "trade_broadcaster.hpp"
#include "redis_pool_singleton.hpp"
#include <spdlog/spdlog.h>
#include <cstdlib>

TradeBroadcaster::TradeBroadcaster() {
    const char* chan = std::getenv("TRADE_CHANNEL");
    redisPubChannel = chan ? chan : "trades_channel";
}

void TradeBroadcaster::publishTrade(const std::string& tradeJson) {
    try {
        redisContext* ctx = RedisPoolSingleton::instance().acquire();
        if (!ctx) {
            SPDLOG_WARN("Redis unavailable — skipping trade publish");
            return;
        }

        SPDLOG_INFO("📢 Publishing trade on '{}': {}", redisPubChannel, tradeJson);

        void* reply = redisCommand(ctx, "PUBLISH %s %s", redisPubChannel.c_str(), tradeJson.c_str());
        if (reply) {
            freeReplyObject(reply);
        } else {
            SPDLOG_ERROR("Redis PUBLISH failed on '{}'", redisPubChannel);
        }

        RedisPoolSingleton::instance().release(ctx);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in TradeBroadcaster::publishTrade: {}", e.what());
    }
}

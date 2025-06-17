#include "order_updater.hpp"
#include "redis_pool_singleton.hpp"
#include <spdlog/spdlog.h>
#include <cstdlib>

OrderUpdater::OrderUpdater() {
    const char* prefix = std::getenv("ORDER_KEY_PREFIX");
    redisKeyPrefix = prefix ? prefix : "order";
}

void OrderUpdater::updateStatus(const std::string& orderId, const std::string& status) {
    try {
        redisContext* ctx = RedisPoolSingleton::instance().acquire();
        if (!ctx) {
            SPDLOG_WARN("Redis context unavailable. Skipping order update.");
            return;
        }

        std::string key = redisKeyPrefix + ":" + orderId;
        SPDLOG_INFO("🔄 Updating order '{}' with status '{}'", key, status);

        void* reply = redisCommand(ctx, "HSET %s status %s", key.c_str(), status.c_str());
        if (reply) {
            freeReplyObject(reply);
        } else {
            SPDLOG_ERROR("Redis HSET failed for key '{}'", key);
        }

        RedisPoolSingleton::instance().release(ctx);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in OrderUpdater::updateStatus: {}", e.what());
    }
}

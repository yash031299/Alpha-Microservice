#include "liquidation_manager.hpp"
#include "position_tracker.hpp"
#include "redis_connection.hpp"
#include <spdlog/spdlog.h>

void rms::LiquidationManager::liquidate(const std::string& userId,
                                   const std::string& symbol,
                                   double positionSize,
                                   int side,  // +1 = long, -1 = short
                                   double ltp) {
    try {
        SPDLOG_WARN("⚙️ LIQUIDATION TRIGGERED for {} @ {:.2f}", userId, ltp);

        // Step 1: Update state in tracker/Redis
        try {
            PositionTracker tracker;
            tracker.updateLiquidatedPosition(userId);
        } catch (const std::exception& ex) {
            SPDLOG_ERROR("❌ Position update failed for {}: {}", userId, ex.what());
        }

        // Step 2: Publish event to Redis stream
        try {
            RedisPublisher redis;
            std::string sideStr = (side == 1 ? "SELL" : "BUY");
            redis.tradeMessage(userId, symbol, ltp, positionSize, sideStr, "LIQUIDATION");
        } catch (const std::exception& ex) {
            SPDLOG_ERROR("❌ Redis trade publish failed for {}: {}", userId, ex.what());
        }

        // Step 3: Final log
        SPDLOG_INFO("💥 Liquidation complete for {} on {}", userId, symbol);

    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("❌ LiquidationManager crashed: {}", ex.what());
    }
}

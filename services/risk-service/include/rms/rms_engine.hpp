#pragma once

#include "rms/user_state.hpp"
#include "rms/metrics.hpp"
#include "redis_retry.hpp"

#include <mutex>
#include <memory>
#include <atomic>
#include <string>

namespace rms {

class RMSEngine {
public:
    explicit RMSEngine(const UserState& userState);

    void onPriceUpdate(double ltp);
    void evaluateRisk();
    void syncMargin();

    double getUnrealizedPnL() const { return unrealizedPnL_; }
    std::string getUserId() const { return state_.userId; }

private:
    UserState state_;
    double lastLTP_ = 0.0;
    double unrealizedPnL_ = 0.0;
    double marginUsed_ = 0.0;
    std::mutex lock_;
    std::atomic<bool> syncingMargin_{false};

    void calculatePnL();
    void checkLiquidation();
};

}  // namespace rms

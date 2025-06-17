#pragma once
#include <atomic>
#include <unordered_map>
#include <string>
#include <mutex>

namespace rms {

class RMSMetrics {
public:
    static RMSMetrics& get() {
        static RMSMetrics instance;
        return instance;
    }

    void reportEquity(const std::string& userId, double equity) {
        std::lock_guard<std::mutex> lock(equityMutex_);
        lastEquity[userId] = equity;
    }

    void incrementLiquidation() {
        liquidationCount.fetch_add(1, std::memory_order_relaxed);
    }

    int getLiquidationCount() const {
        return liquidationCount.load(std::memory_order_relaxed);
    }

    std::unordered_map<std::string, double> getLastEquitySnapshot() const {
        std::lock_guard<std::mutex> lock(equityMutex_);
        return lastEquity;
    }

private:
    RMSMetrics() = default;
    std::atomic<int> liquidationCount{0};
    std::unordered_map<std::string, double> lastEquity;
    mutable std::mutex equityMutex_;
};

}  // namespace rms

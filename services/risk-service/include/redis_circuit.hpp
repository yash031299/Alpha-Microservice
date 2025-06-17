#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <spdlog/spdlog.h>

class RedisCircuitBreaker {
public:
    RedisCircuitBreaker(int failureThreshold, int cooloffSeconds)
        : failureThreshold_(failureThreshold),
          cooloffDuration_(std::chrono::seconds(cooloffSeconds)),
          failureCount_(0),
          isTripped_(false),
          lastTripTime_(std::chrono::steady_clock::now()) {}

    void recordFailure() {
        std::lock_guard<std::mutex> guard(lock_);
        failureCount_++;

        if (failureCount_ >= failureThreshold_ && !isTripped_) {
            isTripped_ = true;
            lastTripTime_ = std::chrono::steady_clock::now();
            SPDLOG_WARN("🚨 Redis circuit breaker TRIPPED!");
        }
    }

    void recordSuccess() {
        std::lock_guard<std::mutex> guard(lock_);
        failureCount_ = 0;
        isTripped_ = false;
    }

    bool isOpen() {
        std::lock_guard<std::mutex> guard(lock_);
        if (!isTripped_) return false;

        auto now = std::chrono::steady_clock::now();
        if (now - lastTripTime_ >= cooloffDuration_) {
            isTripped_ = false;
            failureCount_ = 0;
            SPDLOG_INFO("✅ Redis circuit breaker RESET.");
            return false;
        }

        return true;
    }

private:
    const int failureThreshold_;
    const std::chrono::seconds cooloffDuration_;
    int failureCount_;
    bool isTripped_;
    std::chrono::steady_clock::time_point lastTripTime_;
    std::mutex lock_;
};

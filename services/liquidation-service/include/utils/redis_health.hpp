#pragma once
#include <chrono>
#include <atomic>
#include <mutex>

class RedisHealthMonitor {
public:
    RedisHealthMonitor(int failureThreshold = 5, int cooldownSec = 10)
        : failureThreshold(failureThreshold), cooldownSeconds(cooldownSec),
          failureCount(0), circuitOpenUntil(std::chrono::steady_clock::now()) {}

    void recordFailure() {
        std::lock_guard<std::mutex> lock(mutex_);
        failureCount++;
        if (failureCount >= failureThreshold) {
            circuitOpenUntil = std::chrono::steady_clock::now() + std::chrono::seconds(cooldownSeconds);
        }
    }

    void recordSuccess() {
        std::lock_guard<std::mutex> lock(mutex_);
        failureCount = 0;
    }

    bool isCircuitOpen() {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::chrono::steady_clock::now() < circuitOpenUntil;
    }

private:
    int failureThreshold;
    int cooldownSeconds;
    int failureCount;
    std::chrono::steady_clock::time_point circuitOpenUntil;
    std::mutex mutex_;
};

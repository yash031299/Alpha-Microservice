#pragma once
#include <thread>
#include <chrono>
#include <functional>
#include <random>

namespace RetryUtils {

inline bool retryWithBackoff(std::function<bool()> task,
                             int maxAttempts = 3,
                             int initialDelayMs = 100,
                             int maxDelayMs = 1000) {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        if (task()) return true;

        int delay = std::min(initialDelayMs * (1 << (attempt - 1)), maxDelayMs);
        std::uniform_int_distribution<> jitter(0, delay / 4);
        delay += jitter(gen);

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
    return false;
}

}

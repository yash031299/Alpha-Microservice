#pragma once
#include <thread>
#include <chrono>
#include <cmath>

class Backoff {
public:
    Backoff(int baseMs = 100, int maxMs = 3000, int factor = 2)
        : base_(baseMs), max_(maxMs), factor_(factor), attempts_(0) {}

    void reset() { attempts_ = 0; }

    void wait() {
        int waitTime = std::min(max_, base_ * static_cast<int>(std::pow(factor_, attempts_++)));
        std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    }

private:
    int base_;
    int max_;
    int factor_;
    int attempts_;
};

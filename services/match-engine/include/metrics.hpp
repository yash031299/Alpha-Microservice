#ifndef METRICS_HPP
#define METRICS_HPP

#include <atomic>
#include <mutex>
#include <string>

class Metrics {
public:
    static void incrementTrades(int count = 1);
    static void observeLatency(double milliseconds);
    static std::string render();

private:
    static std::atomic<uint64_t> totalTrades_;
    static std::atomic<uint64_t> totalLatencyMs_;
    static std::atomic<uint64_t> latencySamples_;
};

#endif

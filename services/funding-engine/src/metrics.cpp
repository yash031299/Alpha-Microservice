#include "metrics.hpp"

void Metrics::incrementCounter(const std::string& name) {
    std::lock_guard<std::mutex> lock(mtx_);
    counters_[name]++;
}

void Metrics::setGauge(const std::string& name, double value) {
    std::lock_guard<std::mutex> lock(mtx_);
    gauges_[name] = value;
}

std::unordered_map<std::string, double> Metrics::snapshot() {
    std::lock_guard<std::mutex> lock(mtx_);
    std::unordered_map<std::string, double> all = counters_;
    for (const auto& [key, value] : gauges_) {
        all[key] = value;
    }
    return all;
}

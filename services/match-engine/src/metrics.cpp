#include "metrics.hpp"
#include <sstream>

std::atomic<uint64_t> Metrics::totalTrades_{0};
std::atomic<uint64_t> Metrics::totalLatencyMs_{0};
std::atomic<uint64_t> Metrics::latencySamples_{0};

void Metrics::incrementTrades(int count) {
    totalTrades_ += count;
}

void Metrics::observeLatency(double ms) {
    totalLatencyMs_ += static_cast<uint64_t>(ms);
    latencySamples_ += 1;
}

std::string Metrics::render() {
    std::ostringstream out;
    out << "# HELP match_engine_total_trades Total trades matched\n";
    out << "# TYPE match_engine_total_trades counter\n";
    out << "match_engine_total_trades " << totalTrades_.load() << "\n";

    out << "# HELP match_engine_avg_latency_ms Average match latency in ms\n";
    out << "# TYPE match_engine_avg_latency_ms gauge\n";
    double avg = (latencySamples_ > 0) ? static_cast<double>(totalLatencyMs_.load()) / latencySamples_.load() : 0;
    out << "match_engine_avg_latency_ms " << avg << "\n";

    return out.str();
}

#ifndef METRICS_HPP
#define METRICS_HPP

#include <unordered_map>
#include <mutex>
#include <string>

class Metrics {
public:
    void incrementCounter(const std::string& name);
    void setGauge(const std::string& name, double value);
    std::unordered_map<std::string, double> snapshot();

private:
    std::mutex mtx_;
    std::unordered_map<std::string, double> counters_;
    std::unordered_map<std::string, double> gauges_;
};

#endif // METRICS_HPP

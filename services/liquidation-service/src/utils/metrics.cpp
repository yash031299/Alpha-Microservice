#include "utils/metrics.hpp"
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

std::shared_ptr<prometheus::Registry> Metrics::registry;
prometheus::Counter* Metrics::liquidation_counter = nullptr;
prometheus::Counter* Metrics::redis_error_counter = nullptr;
prometheus::Counter* Metrics::retry_counter = nullptr;

void Metrics::init(int port) {
    static prometheus::Exposer exposer("0.0.0.0:" + std::to_string(port));
    registry = std::make_shared<prometheus::Registry>();

    auto& family1 = prometheus::BuildCounter()
                        .Name("liquidation_total")
                        .Help("Total number of liquidations")
                        .Register(*registry);
    liquidation_counter = &family1.Add({});

    auto& family2 = prometheus::BuildCounter()
                        .Name("redis_errors_total")
                        .Help("Total Redis failures")
                        .Register(*registry);
    redis_error_counter = &family2.Add({});

    auto& family3 = prometheus::BuildCounter()
                        .Name("liquidation_retries_total")
                        .Help("Total Redis retries")
                        .Register(*registry);
    retry_counter = &family3.Add({});

    exposer.RegisterCollectable(registry);
}

prometheus::Counter& Metrics::liquidationCounter() { return *liquidation_counter; }
prometheus::Counter& Metrics::redisErrorCounter() { return *redis_error_counter; }
prometheus::Counter& Metrics::retryCounter() { return *retry_counter; }

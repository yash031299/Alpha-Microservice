#include "metrics.hpp"
#include <prometheus/counter.h>
#include <prometheus/histogram.h>

std::shared_ptr<Metrics> Metrics::init(int port) {
    auto exposer = std::make_shared<prometheus::Exposer>("0.0.0.0:" + std::to_string(port));
    auto registry = std::make_shared<prometheus::Registry>();

    auto& counter_family = prometheus::BuildCounter()
        .Name("order_requests_total")
        .Help("Total number of orders received")
        .Register(*registry);

    auto& rejected_family = prometheus::BuildCounter()
        .Name("order_rejected_total")
        .Help("Number of rejected orders")
        .Register(*registry);

    auto& fallback_family = prometheus::BuildCounter()
        .Name("order_redis_fallbacks")
        .Help("Number of orders pushed to Redis fallback")
        .Register(*registry);

    auto& hist_family = prometheus::BuildHistogram()
        .Name("order_processing_time")
        .Help("Order processing time in ms")
        .Register(*registry);

    exposer->RegisterCollectable(registry);

    return std::make_shared<Metrics>(Metrics{
        registry,
        &counter_family.Add({}),
        &rejected_family.Add({}),
        &fallback_family.Add({}),
        &hist_family.Add({}, prometheus::Histogram::BucketBoundaries{5, 10, 20, 50, 100, 200, 500})
    });
}

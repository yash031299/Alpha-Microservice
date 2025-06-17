#pragma once

#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <memory>

struct Metrics {
    std::shared_ptr<prometheus::Registry> registry;
    prometheus::Counter* totalOrders;
    prometheus::Counter* rejectedOrders;
    prometheus::Counter* redisFallbacks;
    prometheus::Histogram* orderLatency;

    static std::shared_ptr<Metrics> init(int port = 8081);
};

#pragma once
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <memory>

class Metrics {
public:
    static void init(int port);
    static prometheus::Counter& liquidationCounter();
    static prometheus::Counter& redisErrorCounter();
    static prometheus::Counter& retryCounter();

private:
    static std::shared_ptr<prometheus::Registry> registry;
    static prometheus::Counter* liquidation_counter;
    static prometheus::Counter* redis_error_counter;
    static prometheus::Counter* retry_counter;
};

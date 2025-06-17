#include "redis_pool_singleton.hpp"
#include <stdexcept>

std::unique_ptr<RedisPool> RedisPoolSingleton::instance_ = nullptr;

void RedisPoolSingleton::init(const std::string& host, int port, int size) {
    if (!instance_) {
        instance_ = std::make_unique<RedisPool>(host, port, size);
    }
}

RedisPool& RedisPoolSingleton::instance() {
    if (!instance_) {
        throw std::runtime_error("RedisPoolSingleton not initialized.");
    }
    return *instance_;
}

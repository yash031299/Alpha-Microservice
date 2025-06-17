#pragma once
#include "redis_pool.hpp"
#include <memory>

class RedisPoolSingleton {
public:
    static void init(const std::string& host, int port, int size = 4);
    static RedisPool& instance();

private:
    static std::unique_ptr<RedisPool> instance_;
};

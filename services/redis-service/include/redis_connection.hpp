#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include <memory>

class RedisConnection {
public:
    RedisConnection();
    ~RedisConnection();

    redisContext* get();

private:
    redisContext* ctx;
    std::string host;
    int port;

    void connect();  // retry-safe connect
};

#pragma once
#include <string>
#include <hiredis/hiredis.h>

class RedisClient {
public:
    RedisClient(const std::string& host = "localhost", int port = 6379);
    ~RedisClient();

    void lpush(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    void del(const std::string& key);

private:
    redisContext* context;
};

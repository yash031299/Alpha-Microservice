#pragma once

#include <hiredis/hiredis.h>
#include <mutex>
#include <queue>
#include <string>
#include <memory>
#include <condition_variable>

class RedisConnectionPool {
public:
    RedisConnectionPool(const std::string& host, int port, int poolSize);
    std::shared_ptr<redisContext> getConnection();

private:
    std::queue<std::shared_ptr<redisContext>> pool;
    std::mutex mutex;
    std::condition_variable cv;

    std::string host;
    int port;
};

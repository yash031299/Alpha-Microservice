#pragma once

#include <hiredis/hiredis.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <string>

class RedisPool {
public:
    RedisPool(const std::string& host, int port, int poolSize = 4);
    ~RedisPool();

    redisContext* acquire();
    void release(redisContext* ctx);

private:
    std::vector<redisContext*> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int maxSize_;
};

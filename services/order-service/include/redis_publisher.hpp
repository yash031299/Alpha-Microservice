#pragma once
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <hiredis/hiredis.h>

class RedisPublisher {
public:
    RedisPublisher(const std::string& host, int port, const std::string& streamKey);
    ~RedisPublisher();

    void enqueue(const std::string& jsonPayload);  // call this on fallback

private:
    void processQueue();
    redisContext* createConnection();

    std::string redisHost_;
    int redisPort_;
    std::string streamKey_;

    std::queue<std::string> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_;
    std::thread worker_;
};

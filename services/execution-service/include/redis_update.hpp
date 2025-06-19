#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <memory>

class RedisConnectionPool {
public:
    RedisConnectionPool();
    ~RedisConnectionPool();

    void enqueueUpdate(const std::string& order_id,
                       const std::string& status,
                       double filled_qty,
                       double avg_price);

private:
    void workerLoop();
    std::thread worker_;
    std::queue<std::tuple<std::string, std::string, double, double>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
};

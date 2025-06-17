#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

#include "redis_connection_pool.hpp"
#include "config_loader.hpp"
#include "interfaces/iredis_client.hpp"
#include "liquidation_strategy.hpp"

class LiquidationController {
public:
    LiquidationController(std::shared_ptr<RedisConnectionPool> redisPool,
                      std::shared_ptr<IRedisClient> redisClient,
                      std::shared_ptr<ILiquidationStrategy> strategy,
                      int threadCount);

    ~LiquidationController();

    void start();
    void stop();

private:
    void listenLoop();
    void processKey(const std::string& key);
    bool isDuplicate(const std::string& key);

    std::shared_ptr<RedisConnectionPool> redisPool;
    std::unique_ptr<boost::asio::thread_pool> workerPool;
    std::atomic<bool> running;
    std::thread listenerThread;

    std::mutex dedupMutex;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> recent;
    const int cacheExpiryMs = 3000;

    std::string redisQueue;
    std::string redisHost;
    int redisPort;

    std::shared_ptr<IRedisClient> redisClient;
    std::shared_ptr<ILiquidationStrategy> strategy;
    std::mutex execMutex;
};

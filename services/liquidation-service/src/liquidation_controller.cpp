#include "liquidation_controller.hpp"
#include <spdlog/spdlog.h>
#include <hiredis/hiredis.h>

LiquidationController::LiquidationController(std::shared_ptr<RedisConnectionPool> redisPool,
                                             std::shared_ptr<IRedisClient> redisClient,
                                             std::shared_ptr<ILiquidationStrategy> strategy,
                                             int threadCount)
    : redisPool(redisPool), redisClient(redisClient), strategy(strategy), running(false) {
    redisQueue = ConfigLoader::getEnv("REDIS_QUEUE", "QUEUE:LIQUIDATE");
    redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
    redisPort = ConfigLoader::getIntEnv("REDIS_PORT", 6379);
    workerPool = std::make_unique<boost::asio::thread_pool>(threadCount);
    SPDLOG_INFO("🧵 Initialized thread pool with {} workers", threadCount);
}


LiquidationController::~LiquidationController() {
    stop();
}

void LiquidationController::start() {
    running = true;
    listenerThread = std::thread(&LiquidationController::listenLoop, this);
}

void LiquidationController::stop() {
    running = false;
    if (listenerThread.joinable()) listenerThread.join();
    workerPool->join();
    SPDLOG_INFO("🛑 LiquidationController stopped");
}

void LiquidationController::listenLoop() {
    SPDLOG_INFO("🔁 Subscribed to Redis queue: {}", redisQueue);

    redisContext* redis = redisConnect(redisHost.c_str(), redisPort);
    if (!redis || redis->err) {
        SPDLOG_CRITICAL("❌ Failed to connect to Redis in listenLoop");
        return;
    }

    while (running) {
        redisReply* reply = (redisReply*)redisCommand(redis, "BLPOP %s 5", redisQueue.c_str());
        if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
            if (reply) freeReplyObject(reply);
            continue;
        }

        std::string key(reply->element[1]->str);
        freeReplyObject(reply);

        if (!isDuplicate(key)) {
            boost::asio::post(*workerPool, [this, key]() {
                this->processKey(key);
            });
        }
    }

    redisFree(redis);
}

bool LiquidationController::isDuplicate(const std::string& key) {
    std::lock_guard<std::mutex> lock(dedupMutex);
    auto now = std::chrono::steady_clock::now();
    auto it = recent.find(key);
    if (it != recent.end()) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() < cacheExpiryMs) {
            SPDLOG_INFO("🌀 Duplicate key detected within cooldown window: {}", key);
            return true;
        }
    }
    recent[key] = now;
    return false;
}

void LiquidationController::processKey(const std::string& key) {
    try {
        auto result = redisClient->getUser(key);
        if (!result.has_value()) {
            SPDLOG_WARN("❌ Could not load user from Redis: {}", key);
            return;
        }

        auto usr = result.value();
        if (!result.has_value()) {
            SPDLOG_WARN("🚫 No user data found for key: {}", key);
            return;
        }
        std::unique_lock lock(execMutex);
        strategy->apply(*result.value());
        redisClient->saveUser(*usr);

    } catch (const std::exception& e) {
        SPDLOG_ERROR("🔥 processKey exception for key {}: {}", key, e.what());
    } catch (...) {
        SPDLOG_ERROR("🔥 Unknown error in processKey for key {}", key);
    }
}


#pragma once
#include <string>
#include <thread>
#include <atomic>

class RedisListener {
public:
    RedisListener();
    void start();
    void stop();

private:
    void listen();
    void publishMarginCheck(const std::string& userId);

    std::atomic<bool> running{false};
    std::thread listenerThread;

    std::string redisHost;
    int redisPort;
    std::string streamKey;
};

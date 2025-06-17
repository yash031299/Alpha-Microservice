#pragma once
#include <thread>
#include <atomic>
#include <string>

class MarginListener {
public:
    void start();
    void stop();

private:
    void listen(const std::string& redisHost, int redisPort);  // 🔧 Update here
    std::thread listenerThread;
    std::atomic<bool> running{false};
};

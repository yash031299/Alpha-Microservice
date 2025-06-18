#pragma once
#include <thread>
#include <atomic>
#include <string>

class MarginListener {
public:
    void start();
    void stop();

private:
    void listen();
    std::thread listenerThread;
    std::atomic<bool> running{false};
};

#pragma once

class RedisListener {
public:
    void start();
    void stop();

private:
    void listen();
    void publishMarginCheck(const std::string& userId);
    bool running = false;
    std::thread listenerThread;
};

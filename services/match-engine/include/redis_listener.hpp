#ifndef REDIS_LISTENER_HPP
#define REDIS_LISTENER_HPP

#include "matcher.hpp"
#include <string>
#include <thread>
#include <atomic>

class RedisListener {
public:
    RedisListener(const std::string& host, int port, const std::string& streamKey, Matcher& matcher);
    ~RedisListener();
    void start();
    void stop();

private:
    void listen();
    std::atomic<bool> running_;
    std::thread worker_;
    std::string host_;
    int port_;
    std::string streamKey_;
    std::string lastId_;
    Matcher& matcher_;
};

#endif

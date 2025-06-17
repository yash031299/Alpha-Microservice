#pragma once

#include "user_manage.h"
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <hiredis/hiredis.h>

class OrderRouterService {
public:
    OrderRouterService(std::shared_ptr<userManager> userMng);
    ~OrderRouterService();
    void start();
    void stop();

private:
    void listenLoop();
    void processOrder(const std::string& json);
    std::string getEnvOrDefault(const std::string& key, const std::string& defaultVal);

    std::shared_ptr<userManager> userMng;

    redisContext* redis;
    std::atomic<bool> running;
    std::thread listenerThread;

    std::string redisQueueName;
    int redisTimeout;
};

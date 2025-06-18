#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <hiredis/hiredis.h>

class MatchEngineClient;
class RMSClient;

class OrderRouterService {
public:
    OrderRouterService(const std::string& redis_host, int redis_port,
                       std::shared_ptr<MatchEngineClient> matchClient,
                       std::shared_ptr<RMSClient> rmsClient);

    ~OrderRouterService();

    void start();
    void stop();

private:
    void listenLoop();
    void processOrder(const std::string& json);
    std::string getEnvOrDefault(const std::string& key, const std::string& defaultVal);

    std::shared_ptr<MatchEngineClient> matchClient_;
    std::shared_ptr<RMSClient> rmsClient_;

    redisContext* redis_;
    std::atomic<bool> running_;
    std::thread listenerThread_;

    std::string redisQueueName_;
    int redisTimeout_;
};

#pragma once
#include <string>

class OrderUpdater {
public:
    OrderUpdater();  // constructor loads config
    void updateStatus(const std::string& orderId, const std::string& status);

private:
    std::string redisKeyPrefix;
};

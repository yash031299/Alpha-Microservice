#pragma once
#include <string>

class TradeBroadcaster {
public:
    TradeBroadcaster();  // loads topic from env
    void publishTrade(const std::string& tradeJson);

private:
    std::string redisPubChannel;
};

#pragma once
#include <nlohmann/json.hpp>
#include "symbol_client.hpp"
#include "redis_client.hpp"
#include "user_client.hpp"

class OrderHandler {
public:
    OrderHandler();
    bool submitOrder(const nlohmann::json& orderJson);
private:
    bool validateSymbol(const std::string& symbol);
    bool checkMargin(const nlohmann::json& orderJson);
    std::shared_ptr<SymbolClient> symbolClient;
    std::shared_ptr<RedisClient> redisClient;
};

#pragma once
#include <string>
#include <nlohmann/json.hpp>

class RiskClient {
public:
    RiskClient(const std::string& redis_host, int redis_port);

    // Sends margin check and waits for decision (ALLOW / DENY)
    bool checkMargin(const std::string& user_id, const nlohmann::json& orderJson);

private:
    class RedisClient* redis_;
};

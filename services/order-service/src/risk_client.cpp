#include "risk_client.hpp"
#include "redis_client.hpp"
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>

RiskClient::RiskClient(const std::string& redis_host, int redis_port) {
    redis_ = new RedisClient(redis_host, redis_port);
}

bool RiskClient::checkMargin(const std::string& user_id, const nlohmann::json& orderJson) {
    std::string key = "QUEUE:MARGIN:CHECK:" + user_id;
    redis_->lpush(key, orderJson.dump());

    std::string resultKey = "MARGIN:RESULT:" + user_id;
    int waited = 0, timeout = 2000;

    while (waited < timeout) {
        std::string result = redis_->get(resultKey);
        if (!result.empty()) {
            redis_->del(resultKey);
            SPDLOG_INFO("📤 Risk check result for {}: {}", user_id, result);
            return result == "ALLOW";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        waited += 100;
    }

    SPDLOG_WARN("⏳ Risk check timeout for user {}", user_id);
    return false;
}

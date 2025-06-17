#include "redis_user_client.hpp"
#include <spdlog/spdlog.h>
#include <hiredis/hiredis.h>
#include "retry_utils.hpp"
#include "utils/metrics.hpp"

RedisUserClient::RedisUserClient(std::shared_ptr<RedisConnectionPool> pool,
                                 const std::string& host,
                                 int port)
    : pool(pool), host(host), port(port) {}

std::optional<std::shared_ptr<user>> RedisUserClient::getUser(const std::string& key) {
    try {
        if (healthMonitor.isCircuitOpen()) {
            SPDLOG_WARN("⛔ Redis circuit breaker open. Skipping user load.");
            return nullptr;

        }

        auto conn = pool->getConnection();
        std::string redisKey = "USER:" + key;

        redisReply* reply = nullptr;
        bool success = RetryUtils::retryWithBackoff([&]() -> bool {
            auto raw = pool->getConnection();
            if (!raw || !raw.get()) {
                SPDLOG_CRITICAL("🚨 Null Redis connection from pool");
                return false;  // ✅ return false instead of std::nullopt
            }

            redisContext* conn = raw.get();
            reply = (redisReply*)redisCommand(conn, "HGETALL %s", redisKey.c_str());
            Metrics::retryCounter().Increment();
            return reply && reply->type == REDIS_REPLY_ARRAY;
        });


        if (!success) {
            healthMonitor.recordFailure();
            Metrics::redisErrorCounter().Increment();
            SPDLOG_ERROR("❌ Redis command failed after retries for key: {}", redisKey);
            if (reply) freeReplyObject(reply);
            return nullptr;
        } else {
            healthMonitor.recordSuccess();
        }

        if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements % 2 != 0) {
            SPDLOG_ERROR("⚠️ Invalid Redis response for key {}", redisKey);
            Metrics::redisErrorCounter().Increment();
            if (reply) freeReplyObject(reply);
            return nullptr;
        }

        user usr;
        usr.userid = key;

        for (size_t i = 0; i < reply->elements; i += 2) {
            std::string field = reply->element[i]->str;
            std::string value = reply->element[i + 1]->str;

            if (field == "wallet") usr.wallet = std::stod(value);
            else if (field.rfind("pos:", 0) == 0) {
                std::shared_ptr<position> pos = std::make_shared<position>();
                pos->symbol = field.substr(4); // remove 'pos:'
                sscanf(value.c_str(), "%lf,%lf,%lf,%lf,%lf,%lf",
                       &pos->quantity, &pos->avg_price, &pos->leverage,
                       &pos->notional, &pos->pnl, &pos->liquid_price);
                usr.positions[pos->symbol] = pos;
            }
        }

        freeReplyObject(reply);
        auto uptr = std::make_shared<user>();
        *uptr = std::move(usr);
        return uptr;

    } catch (const std::exception& e) {
        SPDLOG_ERROR("🔥 RedisUserClient::getUser exception: {}", e.what());
        return nullptr;
    }
}

bool RedisUserClient::saveUser(const user& usr) {
    try {
        auto conn = pool->getConnection();
        std::string key = "USER:" + usr.userid;

        // Delete existing data to avoid stale values
        redisCommand(conn.get(), "DEL %s", key.c_str());

        // Set wallet
        redisCommand(conn.get(), "HSET %s wallet %f", key.c_str(), usr.wallet);

        // Set each position field
        for (const auto& [symbol, pos] : usr.positions) {
            std::string posStr = fmt::format("{},{},{},{},{},{}",
                pos->quantity, pos->avg_price, pos->leverage,
                pos->notional, pos->pnl, pos->liquid_price
            );
            redisCommand(conn.get(), "HSET %s pos:%s %s", key.c_str(), symbol.c_str(), posStr.c_str());
        }

        SPDLOG_INFO("✅ User saved to Redis: {}", key);
        return true;

    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ RedisUserClient::saveUser exception: {}", e.what());
        return false;
    }
}

#pragma once
#include "interfaces/iredis_client.hpp"
#include "redis_connection_pool.hpp"
#include "utils/redis_health.hpp"

class RedisUserClient : public IRedisClient {
public:
    RedisUserClient(std::shared_ptr<RedisConnectionPool> pool,
                    const std::string& redisHost,
                    int redisPort);

    std::optional<std::shared_ptr<user>> getUser(const std::string& key) override;
    bool saveUser(const user& usr) override;

private:
    std::shared_ptr<RedisConnectionPool> pool;
    std::string host;
    int port;
    RedisHealthMonitor healthMonitor;
};

#ifndef REDIS_PUBLISHER_HPP
#define REDIS_PUBLISHER_HPP

#include <string>
#include <mutex>
#include <hiredis/hiredis.h>

class RedisPublisher {
public:
    RedisPublisher(const std::string& host, int port);
    ~RedisPublisher();

    bool publishSnapshot(const std::string& symbol, const std::string& snapshotJson);

private:
    redisContext* context_;
    std::mutex mutex_;
    std::string host_;
    int port_;

    bool reconnect();
};

#endif // REDIS_PUBLISHER_HPP

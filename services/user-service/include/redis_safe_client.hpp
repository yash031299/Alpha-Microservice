#ifndef REDIS_SAFE_CLIENT_HPP
#define REDIS_SAFE_CLIENT_HPP

#include <string>
#include <memory>
#include <hiredis/hiredis.h>

class RedisSafeClient {
public:
    RedisSafeClient(const std::string& host, int port);
    ~RedisSafeClient();

    /// Connects or reconnects to Redis
    bool connect();

    /// Checks if currently connected
    bool isConnected() const;

    /// Executes a Redis command (printf-style format)
    redisReply* command(const char* format, ...);

private:
    bool connectWithRetry(int maxRetries = 5, int baseDelayMs = 200);
    std::string host_;
    int port_;
    redisContext* ctx_;
};

#endif // REDIS_SAFE_CLIENT_HPP

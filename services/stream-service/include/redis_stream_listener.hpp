#pragma once
#include <string>

class RedisStreamListener {
public:
    RedisStreamListener();
    void run();

private:
    std::string redis_host_;
    int redis_port_;
    std::string stream_key_;
    
    std::string pub_channel_; 
    void loadEnv();
    void listen();
};

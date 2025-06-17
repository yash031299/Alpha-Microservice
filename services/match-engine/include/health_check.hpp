#ifndef HEALTH_CHECK_HPP
#define HEALTH_CHECK_HPP

#include <thread>
#include <atomic>

class HealthCheckServer {
public:
    explicit HealthCheckServer(int port = 8081);
    ~HealthCheckServer();
    void start();
    void stop();

private:
    std::thread serverThread_;
    std::atomic<bool> running_;
    int port_;
};

#endif

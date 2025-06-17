#pragma once
#include <thread>
#include <atomic>
#include "ledger.hpp"
#include <httplib.h>

class MetricsServer {
public:
    MetricsServer(std::shared_ptr<Ledger> ledger, int port);
    void start();
    void stop();

private:
    std::shared_ptr<Ledger> ledger_;
    int port_;
    std::unique_ptr<httplib::Server> server_;
    std::thread server_thread_;
    std::atomic<bool> started_{false};  // ✅ New flag
    std::atomic<bool> running_{false};       // ✅ For metrics loop control
    std::atomic<int> snapshot_count{0};      // ✅ Used in metrics
};

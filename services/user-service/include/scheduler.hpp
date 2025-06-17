#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "user_wallet_store.hpp"
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

class WalletFlushScheduler {
public:
    explicit WalletFlushScheduler(std::shared_ptr<UserWalletStore> walletStore, int intervalSeconds);
    void start();
    void stop();

private:
    std::shared_ptr<UserWalletStore> walletStore_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    int intervalSeconds_;
};

#endif // SCHEDULER_HPP

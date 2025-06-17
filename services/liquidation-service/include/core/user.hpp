#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include "position.hpp"

struct user {
    std::string userid;
    double wallet = 0.0;
    double unrealized_pnl = 0.0;
    double equity = 0.0;
    double maintainance_margin = 0.0;
    double available_margin = 0.0;

    std::unordered_map<std::string, std::shared_ptr<position>> positions;
    std::vector<std::vector<std::string>> orders;
    std::vector<std::vector<std::string>> slorders;

    std::atomic<bool> is_processing = false;
    std::atomic<bool> is_liquidating = false;
    std::mutex ind_lock;


    user& operator=(user&& other) noexcept {
        if (this != &other) {
            userid = std::move(other.userid);
            wallet = other.wallet;
            unrealized_pnl = other.unrealized_pnl;
            equity = other.equity;
            maintainance_margin = other.maintainance_margin;
            available_margin = other.available_margin;
            positions = std::move(other.positions);
            orders = std::move(other.orders);
            slorders = std::move(other.slorders);
            is_processing.store(other.is_processing.load());
            is_liquidating.store(other.is_liquidating.load());
            // Do NOT copy mutex
        }
        return *this;
    }

};


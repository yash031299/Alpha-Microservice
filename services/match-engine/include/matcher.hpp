#ifndef MATCHER_HPP
#define MATCHER_HPP

#include "book.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <spdlog/spdlog.h>


class Matcher {
public:
    std::vector<Trade> submit(const Order& order);
    bool cancel(const std::string& symbol, const std::string& orderId);
    bool modify(const std::string& symbol, const Order& newOrder);


private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<OrderBook>> books_;
};

#endif // MATCHER_HPP

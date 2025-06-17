#include "book.hpp"
#include <chrono>
#include <sstream>
#include <spdlog/spdlog.h>

OrderBook::OrderBook(const std::string& symbol)
    : symbol_(symbol) {}

bool OrderBook::cancelOrder(const std::string& orderId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (orderIndex_.find(orderId) == orderIndex_.end()) return false;

    auto [isBuy, price] = orderIndex_[orderId];

    bool erased = false;
    if (isBuy) {
        auto it = bids_.find(price);
        if (it != bids_.end()) {
            auto& queue = it->second;
            for (auto qIt = queue.begin(); qIt != queue.end(); ++qIt) {
                if (qIt->orderId == orderId) {
                    queue.erase(qIt);
                    if (queue.empty()) bids_.erase(price);
                    erased = true;
                    break;
                }
            }
        }
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end()) {
            auto& queue = it->second;
            for (auto qIt = queue.begin(); qIt != queue.end(); ++qIt) {
                if (qIt->orderId == orderId) {
                    queue.erase(qIt);
                    if (queue.empty()) asks_.erase(price);
                    erased = true;
                    break;
                }
            }
        }
    }

    if (erased) orderIndex_.erase(orderId);
    return erased;
}

std::vector<Trade> OrderBook::processOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Trade> trades;

    try {
        if (order.quantity <= 0 || order.price < 0) {
            SPDLOG_WARN("⚠️ Invalid order values: price={}, quantity={}", order.price, order.quantity);
            return trades;
        }

        auto incoming = order;
        std::function<bool(double, double)> comparator;
        if (incoming.isBuy)
            comparator = [](double a, double b) { return a <= b; };
        else
            comparator = [](double a, double b) { return a >= b; };


        if (incoming.isBuy) {
            for (auto it = asks_.begin(); it != asks_.end() && incoming.quantity > 0;) {
                double levelPrice = it->first;
                if (!comparator(incoming.price, levelPrice)) break;

                auto& queue = it->second;
                while (!queue.empty() && incoming.quantity > 0) {
                    Order& resting = queue.front();
                    double tradedQty = std::min(incoming.quantity, resting.quantity);

                    trades.push_back({
                        .makerOrderId = resting.orderId,
                        .takerOrderId = incoming.orderId,
                        .symbol = symbol_,
                        .price = levelPrice,
                        .quantity = tradedQty,
                        .timestamp = "TODO_NOW"
                    });

                    resting.quantity -= tradedQty;
                    incoming.quantity -= tradedQty;

                    if (resting.quantity <= 0) {
                        orderIndex_.erase(resting.orderId);
                        queue.pop_front();
                    }
                }
                if (queue.empty()) it = asks_.erase(it);
                else ++it;
            }
        } else {
            for (auto it = bids_.begin(); it != bids_.end() && incoming.quantity > 0;) {
                double levelPrice = it->first;
                if (!comparator(incoming.price, levelPrice)) break;

                auto& queue = it->second;
                while (!queue.empty() && incoming.quantity > 0) {
                    Order& resting = queue.front();
                    double tradedQty = std::min(incoming.quantity, resting.quantity);

                    trades.push_back({
                        .makerOrderId = resting.orderId,
                        .takerOrderId = incoming.orderId,
                        .symbol = symbol_,
                        .price = levelPrice,
                        .quantity = tradedQty,
                        .timestamp = "TODO_NOW"
                    });

                    resting.quantity -= tradedQty;
                    incoming.quantity -= tradedQty;

                    if (resting.quantity <= 0) {
                        orderIndex_.erase(resting.orderId);
                        queue.pop_front();
                    }
                }
                if (queue.empty()) it = bids_.erase(it);
                else ++it;
            }
        }

        if (incoming.tif == TimeInForce::IOC) return trades;
        if (incoming.tif == TimeInForce::FOK && incoming.quantity > 0) return {};

        if (incoming.quantity > 0) {
            if (incoming.isBuy)
                bids_[incoming.price].push_back(incoming);
            else
                asks_[incoming.price].push_back(incoming);

            orderIndex_[incoming.orderId] = {incoming.isBuy, incoming.price};
            SPDLOG_INFO("📥 Order {} stored on book: qty={}", incoming.orderId, incoming.quantity);
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("💥 Exception in processOrder(): {}", e.what());
    } catch (...) {
        SPDLOG_ERROR("💥 Unknown error in processOrder()");
    }

    return trades;
}

bool OrderBook::modifyOrder(const std::string& orderId, double newPrice, double newQuantity) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        if (orderIndex_.find(orderId) == orderIndex_.end()) return false;

        auto [isBuy, oldPrice] = orderIndex_[orderId];

        if (isBuy) {
            auto it = bids_.find(oldPrice);
            if (it == bids_.end()) return false;

            auto& queue = it->second;
            for (auto qIt = queue.begin(); qIt != queue.end(); ++qIt) {
                if (qIt->orderId == orderId) {
                    Order updated = *qIt;
                    queue.erase(qIt);
                    if (queue.empty()) bids_.erase(oldPrice);

                    updated.price = newPrice;
                    updated.quantity = newQuantity;

                    bids_[newPrice].push_back(updated);
                    orderIndex_[orderId] = {isBuy, newPrice};
                    return true;
                }
            }
        } else {
            auto it = asks_.find(oldPrice);
            if (it == asks_.end()) return false;

            auto& queue = it->second;
            for (auto qIt = queue.begin(); qIt != queue.end(); ++qIt) {
                if (qIt->orderId == orderId) {
                    Order updated = *qIt;
                    queue.erase(qIt);
                    if (queue.empty()) asks_.erase(oldPrice);

                    updated.price = newPrice;
                    updated.quantity = newQuantity;

                    asks_[newPrice].push_back(updated);
                    orderIndex_[orderId] = {isBuy, newPrice};
                    return true;
                }
            }
        }

        return false;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ modifyOrder failed for {}: {}", orderId, e.what());
        return false;
    }
}

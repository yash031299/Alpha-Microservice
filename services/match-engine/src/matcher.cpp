#include "matcher.hpp"

std::vector<Trade> Matcher::submit(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        auto& book = books_[order.symbol];

        if (!book) {
            SPDLOG_INFO("📘 Creating new book for symbol {}", order.symbol);
            book = std::make_shared<OrderBook>(order.symbol);
        }

        SPDLOG_INFO("📩 Submitting order {} for {}", order.orderId, order.symbol);
        return book->processOrder(order);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ Error submitting order {}: {}", order.orderId, e.what());
        return {};
    }
}


bool Matcher::cancel(const std::string& symbol, const std::string& orderId) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        auto it = books_.find(symbol);
        if (it == books_.end() || !it->second) {
            SPDLOG_WARN("⚠️ Cancel failed: Book not found for symbol {}", symbol);
            return false;
        }

        bool result = it->second->cancelOrder(orderId);
        if (!result)
            SPDLOG_WARN("⚠️ Cancel failed: Order {} not found in {}", orderId, symbol);
        else
            SPDLOG_INFO("🗑️ Cancelled order {} from {}", orderId, symbol);

        return result;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ Error cancelling order {}: {}", orderId, e.what());
        return false;
    }
}


bool Matcher::modify(const std::string& symbol, const Order& newOrder) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = books_.find(symbol);
    if (it == books_.end()) {
        SPDLOG_WARN("⚠️ Modify failed: symbol {} not found", symbol);
        return false;
    }

    auto& book = it->second;
    bool cancelSuccess = book->cancelOrder(newOrder.orderId);
    if (!cancelSuccess) {
        SPDLOG_WARN("⚠️ Modify failed: order {} not found", newOrder.orderId);
        return false;
    }

    book->processOrder(newOrder);
    SPDLOG_INFO("✏️ Modified order {} for symbol {}", newOrder.orderId, symbol);
    return true;
}


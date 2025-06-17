#include "orderbook_manager.hpp"
#include <spdlog/spdlog.h>

std::shared_ptr<OrderBookManager::Book> OrderBookManager::getBook(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(booksMutex_);
    auto it = books_.find(symbol);
    if (it == books_.end()) {
        auto book = std::make_shared<Book>();
        books_[symbol] = book;
        return book;
    }
    return it->second;
}

void OrderBookManager::addOrder(const std::string& symbol, const Order& order) {
    try {
        auto book = getBook(symbol);
        std::lock_guard<std::mutex> lock(book->mutex);

        book->orders[order.orderId] = order;
        if (order.isBuy) 
            book->bids[order.price] += order.quantity;
        else
            book->asks[order.price] += order.quantity;

        SPDLOG_INFO("🟢 Order added [{}]: ID={}, price={}, qty={}", symbol, order.orderId, order.price, order.quantity);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("addOrder failed for {}: {}", symbol, e.what());
    }
}

void OrderBookManager::cancelOrder(const std::string& symbol, const std::string& orderId) {
    try {
        auto book = getBook(symbol);
        std::lock_guard<std::mutex> lock(book->mutex);

        auto it = book->orders.find(orderId);
        if (it == book->orders.end()) {
            SPDLOG_WARN("cancelOrder: Order not found [{}] → {}", symbol, orderId);
            return;
        }

        const auto& order = it->second;
        if (order.isBuy) {
            book->bids[order.price] += order.quantity;
            if (book->bids[order.price] <= 0) book->bids.erase(order.price);
        } else {
            book->asks[order.price] += order.quantity;
            if (book->asks[order.price] <= 0) book->asks.erase(order.price);
        }

        SPDLOG_INFO("🔴 Order canceled [{}]: {}", symbol, orderId);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("cancelOrder failed for {}: {}", symbol, e.what());
    }
}

void OrderBookManager::modifyOrder(const std::string& symbol, const Order& order) {
    cancelOrder(symbol, order.orderId);
    addOrder(symbol, order);
}

std::vector<OrderLevel> OrderBookManager::getTopOfBook(const std::string& symbol, size_t depth) {
    std::vector<OrderLevel> snapshot;

    try {
        auto book = getBook(symbol);
        std::lock_guard<std::mutex> lock(book->mutex);

        size_t count = 0;
        for (const auto& [price, qty] : book->bids) {
            snapshot.push_back({price, qty});
            if (++count >= depth) break;
        }

        count = 0;
        for (const auto& [price, qty] : book->asks) {
            snapshot.push_back({price, qty});
            if (++count >= depth) break;
        }

    } catch (const std::exception& e) {
        SPDLOG_ERROR("getTopOfBook failed for {}: {}", symbol, e.what());
    }

    return snapshot;
}

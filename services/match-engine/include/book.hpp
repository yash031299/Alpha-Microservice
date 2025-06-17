#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <map>
#include <deque>
#include <string>
#include <vector>
#include <optional>
#include <mutex>

enum class OrderType { LIMIT, MARKET, STOP_LIMIT };
enum class TimeInForce { GTC, IOC, FOK };

struct Order {
    std::string orderId;
    std::string userId;
    std::string symbol;
    double price;
    double quantity;
    bool isBuy;

    // NEW fields
    OrderType orderType = OrderType::LIMIT;
    TimeInForce tif = TimeInForce::GTC;
    double triggerPrice = 0;
    bool reduceOnly = false;
    bool postOnly = false;
};


struct Trade {
    std::string makerOrderId;
    std::string takerOrderId;
    std::string symbol;
    double price;
    double quantity;
    std::string timestamp;
};

class OrderBook {
public:
    OrderBook(const std::string& symbol);

    std::vector<Trade> processOrder(const Order& order);
    bool cancelOrder(const std::string& orderId);
    bool modifyOrder(const std::string& orderId, double newPrice, double newQuantity);


private:
    std::string symbol_;
    std::mutex mutex_;

    std::map<double, std::deque<Order>, std::greater<>> bids_;
    std::map<double, std::deque<Order>> asks_;
    std::map<std::string, std::pair<bool, double>> orderIndex_; // orderId -> (isBuy, price)

    std::vector<Trade> match(Order incoming);
};

#endif // ORDER_BOOK_HPP

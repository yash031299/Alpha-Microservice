#include "../include/book.hpp"
#include <cassert>
#include <iostream>

void test_limit_match() {
    OrderBook book("BTC-USDT");

    Order ask = { "ask-1", "user-A", "BTC-USDT", 30000, 1.0, false };
    Order bid = { "bid-1", "user-B", "BTC-USDT", 30000, 1.0, true };

    book.processOrder(ask);
    auto trades = book.processOrder(bid);

    assert(trades.size() == 1);
    std::cout << "✅ test_limit_match passed\n";
}

void test_ioc_rejection() {
    OrderBook book("BTC-USDT");

    Order o = { "ioc-1", "user-X", "BTC-USDT", 31000, 1.0, true };
    o.tif = TimeInForce::IOC;

    auto trades = book.processOrder(o);
    assert(trades.size() == 0); // nothing to fill
    std::cout << "✅ test_ioc_rejection passed\n";
}

void test_fok_rejection() {
    OrderBook book("BTC-USDT");

    Order o = { "fok-1", "user-Y", "BTC-USDT", 31000, 2.0, true };
    o.tif = TimeInForce::FOK;

    Order ask = { "ask-2", "user-Z", "BTC-USDT", 31000, 1.0, false };
    book.processOrder(ask);

    auto trades = book.processOrder(o);
    assert(trades.empty()); // not fully filled
    std::cout << "✅ test_fok_rejection passed\n";
}

void test_modify_order() {
    OrderBook book("BTC-USDT");

    Order orig = { "mod-1", "user-A", "BTC-USDT", 30000, 1.0, true };
    book.processOrder(orig);

    book.cancelOrder("mod-1");

    Order mod = { "mod-1", "user-A", "BTC-USDT", 29999, 2.0, true };
    book.processOrder(mod);

    auto trades = book.processOrder({ "taker", "user-B", "BTC-USDT", 29999, 2.0, false });
    assert(trades.size() == 1);
    std::cout << "✅ test_modify_order passed\n";
}

int main() {
    test_limit_match();
    test_ioc_rejection();
    test_fok_rejection();
    test_modify_order();
    return 0;
}

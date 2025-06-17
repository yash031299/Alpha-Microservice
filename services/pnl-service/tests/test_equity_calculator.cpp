#include "../include/equity_calculator.hpp"
#include <cassert>
#include <iostream>
#include "symbol.grpc.pb.h"


// Dummy mock class
class MockSymbolClient : public SymbolServiceClient {
public:
    MockSymbolClient() : SymbolServiceClient("mock") {}

    symbol::SymbolResponse getSymbolMetadata(const std::string& symbol) {
        symbol::SymbolResponse res;
        res.set_found(true);
        res.set_symbol(symbol);
        res.set_tick_size(0.01);
        res.set_max_leverage(10);
        res.set_price_precision(2);
        res.set_quantity_precision(2);
        return res;
    }

};

void testEquityCalculator() {
    EquityCalculator eq("user-1", 1000.0);
    eq.setSymbolService(std::make_shared<MockSymbolClient>());

    // Buy 1 ETH at 2000
    eq.updatePosition("ETH-USDT", 1.0, 2000.0);
    eq.updateLTP("ETH-USDT", 2100.0);

    double equity = eq.getEquity();
    assert(equity == 1100.0);

    bool liquid = eq.isLiquidatable();  // not liquidatable if 1% of 2100 = 21 < 1100
    assert(!liquid);

    std::cout << "✅ EquityCalculator unit test passed.\n";
}

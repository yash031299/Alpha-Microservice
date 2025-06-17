#include "../include/pnl_engine.hpp"
#include <cassert>
#include <iostream>

void testPnLEngine() {
    PnLEngine engine("test-user", 1000.0);
    
    // Buy 2 BTC at $30,000
    engine.updatePosition("BTC-USDT", 2.0, 30000.0);
    
    // LTP = 31,000 → unrealized profit = (31000 - 30000) * 2 = 2000
    engine.onPriceUpdate("BTC-USDT", 31000.0);
    
    auto pnl = engine.computePnL();
    assert(pnl.unrealized == 2000.0);
    assert(pnl.equity == 1200.0 + 800.0);  // wallet + unrealized
    
    // Sell 2 BTC at 32,000 → realized = 4000
    engine.updatePosition("BTC-USDT", -2.0, 32000.0);
    pnl = engine.computePnL();
    assert(pnl.unrealized == 0.0);
    assert(pnl.realized == 4000.0);  // tracked in wallet
    assert(pnl.equity == 1000.0 + 4000.0);

    std::cout << "✅ PnLEngine unit test passed.\n";
}

int main() {
    testPnLEngine();
    return 0;
}

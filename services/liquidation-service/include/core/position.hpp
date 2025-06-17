#pragma once
#include <string>

struct position {
    std::string symbol;
    double quantity = 0.0;
    double avg_price = 0.0;
    double leverage = 1.0;
    double notional = 0.0;
    double pnl = 0.0;
    double liquid_price = 0.0;
};

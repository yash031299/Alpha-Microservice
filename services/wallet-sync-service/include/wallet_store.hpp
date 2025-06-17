#pragma once
#include <string>

class WalletStore {
public:
    void applyTrade(const std::string& userId, double pnl);
};

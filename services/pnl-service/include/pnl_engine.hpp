#ifndef PNL_ENGINE_HPP
#define PNL_ENGINE_HPP

#include <string>
#include <unordered_map>
#include <mutex>

struct Position {
    double quantity = 0.0;     // Positive: long, Negative: short
    double avgEntry = 0.0;     // Average entry price
};

struct PnLResult {
    double unrealized = 0.0;
    double realized = 0.0;
    double equity = 0.0;
};

class PnLEngine {
public:
    PnLEngine(const std::string& userId, double wallet);

    void updatePosition(const std::string& symbol, double qty, double entryPrice);
    void onPriceUpdate(const std::string& symbol, double ltp);

    void addToWallet(double delta);
    PnLResult computePnL();
    const std::unordered_map<std::string, Position>& getOpenPositions() const { return positions_; }
    double getLTP(const std::string& symbol) const {
        auto it = ltps_.find(symbol);
        return (it != ltps_.end()) ? it->second : 0.0;
    }

private:
    std::string userId_;
    double wallet_;
    double realized_;
    std::unordered_map<std::string, Position> positions_;
    std::unordered_map<std::string, double> ltps_;
    std::mutex mutex_;

    double computeUnrealized(const Position& pos, double ltp);
    
};

#endif // PNL_ENGINE_HPP

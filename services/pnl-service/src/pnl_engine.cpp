#include "pnl_engine.hpp"
#include "realized_pnl_store.hpp"
#include <chrono>
#include <iomanip>


PnLEngine::PnLEngine(const std::string& userId, double wallet)
    : userId_(userId), wallet_(wallet), realized_(0.0) {}

void PnLEngine::addToWallet(double delta) {
    std::lock_guard<std::mutex> lock(mutex_);
    wallet_ += delta;
}

void PnLEngine::updatePosition(const std::string& symbol, double qty, double entryPrice) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& pos = positions_[symbol];

    double totalQty = pos.quantity + qty;
    if (totalQty == 0) {
        // Closing position
        double exitPnl = pos.quantity * (entryPrice - pos.avgEntry);
        realized_ += exitPnl;
        wallet_ += exitPnl;
        pos.quantity = 0;
        pos.avgEntry = 0;

        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ts;
        ts << std::put_time(std::gmtime(&t), "%FT%TZ");

        RealizedPnlStore::getInstance().appendPnl(userId_, symbol, exitPnl, ts.str());
        return;
    }

    // Weighted average update for partial adds
    pos.avgEntry = (pos.quantity * pos.avgEntry + qty * entryPrice) / totalQty;
    pos.quantity = totalQty;
}


void PnLEngine::onPriceUpdate(const std::string& symbol, double ltp) {
    std::lock_guard<std::mutex> lock(mutex_);
    ltps_[symbol] = ltp;
}

double PnLEngine::computeUnrealized(const Position& pos, double ltp) {
    if (pos.quantity == 0) return 0.0;
    return (ltp - pos.avgEntry) * pos.quantity;
}

PnLResult PnLEngine::computePnL() {
    std::lock_guard<std::mutex> lock(mutex_);
    double unreal = 0.0;
    for (const auto& [symbol, pos] : positions_) {
        if (ltps_.count(symbol))
            unreal += computeUnrealized(pos, ltps_[symbol]);
    }

    return {
        .unrealized = unreal,
        .realized = realized_,
        .equity = wallet_ + unreal
    };
}

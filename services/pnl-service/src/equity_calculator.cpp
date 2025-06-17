#include "equity_calculator.hpp"
#include <spdlog/spdlog.h>

EquityCalculator::EquityCalculator(const std::string& userId, double wallet)
    : userId_(userId), engine_(userId, wallet) {}

void EquityCalculator::setSymbolService(std::shared_ptr<SymbolServiceClient> symbolClient) {
    symbolClient_ = std::move(symbolClient);
}

void EquityCalculator::updateLTP(const std::string& symbol, double ltp) {
    engine_.onPriceUpdate(symbol, ltp);
}

void EquityCalculator::updatePosition(const std::string& symbol, double qty, double entryPrice) {
    engine_.updatePosition(symbol, qty, entryPrice);
}

double EquityCalculator::getEquity() {
    return engine_.computePnL().equity;
}

bool EquityCalculator::isLiquidatable() {
    auto pnl = engine_.computePnL();

    double totalRequiredMargin = 0.0;

    try {
        const auto& positions = engine_.getOpenPositions();

        for (const auto& [symbol, pos] : positions) {
            if (std::abs(pos.quantity) < 1e-8) continue;

            try {
                auto meta = symbolClient_->getSymbolMetadata(symbol);
                double mmr = 0.003;
                double notional = std::abs(pos.quantity * engine_.getLTP(symbol));
                totalRequiredMargin += mmr * notional;
            } catch (const std::exception& e) {
                SPDLOG_WARN("⚠️ Failed to fetch margin for {}: {}. Using fallback.", symbol, e.what());
                constexpr double fallbackMMR = 0.003;
                double notional = std::abs(pos.quantity * engine_.getLTP(symbol));
                totalRequiredMargin += fallbackMMR * notional;
            }
        }

        return pnl.equity < totalRequiredMargin;

    } catch (const std::exception& e) {
        SPDLOG_ERROR("Failed to compute liquidation check: {}", e.what());
        return false;  // safe default
    }
}
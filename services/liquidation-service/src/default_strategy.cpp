#include "liquidation_strategy.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>
#include "utils/metrics.hpp"
#include <unordered_set>

namespace {
    constexpr double mmr = 0.003;
    constexpr double liquidation_factor = 0.1;
}

class DefaultLiquidationStrategy : public ILiquidationStrategy {
public:
    void apply(user& usr) override {
        if (usr.is_liquidating.exchange(true)) {
            SPDLOG_WARN("⚠️ Liquidation already in progress for user {}. Skipping.", usr.userid);
            return;
        }

        SPDLOG_INFO("🔍 Starting liquidation for user: {}", usr.userid);

        // Step 1: Collect and sort positions by worst PnL
        std::vector<std::pair<std::string, std::shared_ptr<position>>> sorted_positions;
        for (const auto& [symbol, pos] : usr.positions) {
            if (pos && std::abs(pos->quantity) > 0.0) {
                sorted_positions.emplace_back(symbol, pos);
            }
        }

        std::sort(sorted_positions.begin(), sorted_positions.end(),
                [](const auto& a, const auto& b) {
                    return a.second->pnl < b.second->pnl;
                });

        std::unordered_set<std::string> toErase;

        // Step 2: Liquidate until equity exceeds threshold
        for (auto& [symbol, pos] : sorted_positions) {
            if (!pos || std::abs(pos->quantity) < 1e-8) continue;

            SPDLOG_INFO("💥 Liquidating symbol: {}, qty = {}, PnL = {}",
                        pos->symbol, pos->quantity, pos->pnl);

            double realizedPnl = -pos->pnl;
            double liqFee = pos->notional * 0.005;

            usr.wallet += realizedPnl;
            usr.wallet -= liqFee;

            SPDLOG_INFO("💸 RealizedPnL: {}, Fee: {}, Updated Wallet: {}", realizedPnl, liqFee, usr.wallet);

            if (usr.wallet < 0) {
                SPDLOG_WARN("🛑 Wallet negative after liquidation: {}", usr.wallet);
            }

            // Mark for removal
            toErase.insert(symbol);

            // Reset position
            pos->quantity = 0.0;
            pos->notional = 0.0;
            pos->pnl = 0.0;

            // Recompute totals
            double total_notional = 0.0;
            usr.unrealized_pnl = 0.0;
            for (const auto& [_, p] : usr.positions) {
                if (p && std::abs(p->quantity) > 0.0) {
                    usr.unrealized_pnl += p->pnl;
                    total_notional += p->notional;
                }
            }

            usr.maintainance_margin = total_notional * mmr;
            double equity = usr.wallet + usr.unrealized_pnl;

            SPDLOG_INFO("📊 Wallet: {}, Equity: {}, Maintenance Margin: {}", usr.wallet, equity, usr.maintainance_margin);

            if (equity >= (1 + liquidation_factor) * usr.maintainance_margin) {
                SPDLOG_INFO("✅ User {} is now healthy. Stopping liquidation.", usr.userid);
                break;
            }
        }

        // Step 3: Cleanup removed positions
        for (const auto& sym : toErase) {
            usr.positions.erase(sym);
        }

        usr.is_liquidating.store(false);
        SPDLOG_INFO("🏁 Liquidation complete for user: {}", usr.userid);
        Metrics::liquidationCounter().Increment();
    }

};


// Factory
std::shared_ptr<ILiquidationStrategy> createDefaultStrategy() {
    return std::make_shared<DefaultLiquidationStrategy>();
}

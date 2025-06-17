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

        // Step 1: Filter and sort positions with non-zero quantity
        std::vector<std::pair<std::string, std::shared_ptr<position>>> sorted_positions;
        for (const auto& [symbol, pos] : usr.positions) {
            if (pos && pos->quantity != 0.0) {
                sorted_positions.emplace_back(symbol, pos);
            }
        }

        std::sort(sorted_positions.begin(), sorted_positions.end(),
                  [](const auto& a, const auto& b) {
                      return a.second->pnl < b.second->pnl;
                  });

        std::unordered_set<std::string> toErase;

        // Step 2: Liquidate until equity is healthy
        for (auto& [symbol, pos] : sorted_positions) {
            if (!pos || pos->quantity == 0.0) continue;

            SPDLOG_INFO("💥 Liquidating symbol: {}, qty = {}, PnL = {}",
                        pos->symbol, pos->quantity, pos->pnl);

            double realizedPnl = -pos->pnl;  // Core formula from gist and monolith
            usr.wallet += realizedPnl;

            // Reset position
            pos->quantity = 0.0;
            pos->notional = 0.0;
            pos->pnl = 0.0;

            toErase.insert(symbol);

            // Recalculate equity and margin
            double total_notional = 0.0;
            usr.unrealized_pnl = 0.0;
            for (const auto& [_, p] : usr.positions) {
                if (p && p->quantity != 0.0) {
                    usr.unrealized_pnl += p->pnl;
                    total_notional += p->notional;
                }
            }

            usr.maintainance_margin = total_notional * mmr;
            double equity = usr.wallet + usr.unrealized_pnl;

            SPDLOG_INFO("🧾 Wallet: {}, Equity: {}, Maintenance: {}", usr.wallet, equity, usr.maintainance_margin);

            if (equity >= (1 + liquidation_factor) * usr.maintainance_margin) {
                SPDLOG_INFO("✅ User {} now healthy. Exiting liquidation.", usr.userid);
                break;
            }
        }

        // Remove dead positions
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

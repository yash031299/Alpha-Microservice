#include "funding_scheduler.hpp"
#include <chrono>
#include <spdlog/spdlog.h>

FundingScheduler::FundingScheduler(boost::asio::io_context& io,
                                   std::shared_ptr<FundingCalculator> calculator,
                                   std::function<void(const std::string&, const FundingRate&)> callback,
                                   int intervalSeconds)
    : io_(io), calculator_(calculator), callback_(callback), interval_(intervalSeconds) {}

void FundingScheduler::addSymbol(const std::string& symbol) {
    timers_[symbol] = std::make_shared<boost::asio::steady_timer>(io_);
}

void FundingScheduler::start() {
    for (auto& [symbol, _] : timers_) {
        scheduleNext(symbol);
    }
}

void FundingScheduler::scheduleNext(const std::string& symbol) {
    auto timer = timers_[symbol];
    timer->expires_after(std::chrono::seconds(interval_));

    timer->async_wait([this, symbol](const boost::system::error_code& ec) {
        if (ec) {
            SPDLOG_WARN("⏱️ Timer error for {}: {}", symbol, ec.message());
            scheduleNext(symbol);  // ensure continuity
            return;
        }

        try {
            FundingRate rate = calculator_->computeFunding(symbol);
            callback_(symbol, rate);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("⚠️ Funding computation failed for {}: {}", symbol, e.what());
        } catch (...) {
            SPDLOG_ERROR("⚠️ Unknown error during funding computation for {}", symbol);
        }

        scheduleNext(symbol);  // always reschedule
    });
}

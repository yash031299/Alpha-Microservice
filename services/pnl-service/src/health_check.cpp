#include <httplib.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <atomic>

static std::atomic<int> tradeUpdates{0};
static std::atomic<int> ltpUpdates{0};

void incrementTradeCount() { ++tradeUpdates; }
void incrementLTPCount()   { ++ltpUpdates; }

void startHealthAndMetricsServer() {
    std::thread([] {
        httplib::Server server;

        server.Get("/healthz", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("OK", "text/plain");
        });

        server.Get("/metrics", [](const httplib::Request&, httplib::Response& res) {
            std::ostringstream metrics;
            metrics << "# HELP pnl_trade_updates Total trade updates received\n";
            metrics << "# TYPE pnl_trade_updates counter\n";
            metrics << "pnl_trade_updates " << tradeUpdates.load() << "\n";

            metrics << "# HELP pnl_ltp_updates Total LTP updates\n";
            metrics << "# TYPE pnl_ltp_updates counter\n";
            metrics << "pnl_ltp_updates " << ltpUpdates.load() << "\n";

            res.set_content(metrics.str(), "text/plain");
        });

        SPDLOG_INFO("📈 Metrics + health server started on port 8090");
        server.listen("0.0.0.0", 8090);
    }).detach();
}

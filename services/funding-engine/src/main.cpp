#include "funding_calculator.hpp"
#include "funding_scheduler.hpp"
#include "funding_publisher.hpp"
#include "config_loader.hpp"

#include <boost/asio.hpp>
#include <csignal>
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
#include <sstream>
#include <atomic>
#include "health_check.hpp"


std::atomic<bool> keepRunning{true};

void signalHandler(int signum) {
    SPDLOG_WARN("⚠️ Signal {} received, stopping Funding Engine...", signum);
    keepRunning = false;
}

std::vector<std::string> splitSymbols(const std::string& csv) {
    std::stringstream ss(csv);
    std::string token;
    std::vector<std::string> result;

    while (std::getline(ss, token, ',')) {
        if (!token.empty()) result.push_back(token);
    }
    return result;
}

int main() {
    try {
        ConfigLoader::loadEnv(".env");

        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

        startHealthCheckServer();

        SPDLOG_INFO("🚀 Starting Funding Engine...");

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        int intervalSec = std::stoi(ConfigLoader::getEnv("FUNDING_INTERVAL", "5"));
        std::string symbolCsv = ConfigLoader::getEnv("SYMBOLS", "BTC-USDT,ETH-USDT");

        std::vector<std::string> symbols = splitSymbols(symbolCsv);

        boost::asio::io_context io;
        auto calculator = std::make_shared<FundingCalculator>(0.01);
        FundingPublisher publisher;

        FundingScheduler scheduler(io, calculator,
            [&](const std::string& symbol, const FundingRate& rate) {
                try {
                    publisher.publish(symbol, rate);
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("Failed to publish funding for {}: {}", symbol, e.what());
                }
            }, intervalSec
        );

        for (const auto& symbol : symbols) {
            scheduler.addSymbol(symbol);

            // TODO: Replace with real OI fetching
            calculator->updateOpenInterest(symbol, 1000.0, 900.0);  // Mock
        }

        scheduler.start();

        std::thread runner([&io]() {
            try {
                io.run();
            } catch (const std::exception& e) {
                SPDLOG_CRITICAL("IO context crashed: {}", e.what());
            }
        });

        while (keepRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        SPDLOG_INFO("🛑 Funding Engine shutting down...");
        io.stop();
        runner.join();
        SPDLOG_INFO("✅ Clean shutdown complete.");

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("Unhandled startup error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

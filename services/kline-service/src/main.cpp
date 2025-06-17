#include "candle_aggregator.hpp"
#include "kline_scheduler.hpp"
#include "config_loader.hpp"
#include "health_check.hpp"
#include <hiredis/hiredis.h>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <sstream>
#include <iomanip>

std::atomic<int> flushCount{0};
std::atomic<double> lastFlushDuration{0.0};
std::atomic<bool> running{true};

boost::asio::io_context* global_io = nullptr; // Used in signal handler

void signalHandler(int signal) {
    SPDLOG_WARN("🔌 Signal received: {}. Shutting down...", signal);
    running = false;
    if (global_io) {
        global_io->stop();  // ✅ Cleanly stop Boost Asio timers
    }
}

TradeTick parseTrade(const redisReply* reply) {
    TradeTick tick;
    try {
        for (size_t i = 0; i + 1 < reply->elements; i += 2) {
            std::string key = reply->element[i]->str;
            std::string val = reply->element[i + 1]->str;

            if (key == "symbol") tick.symbol = val;
            else if (key == "price") tick.price = std::stod(val);
            else if (key == "quantity") tick.quantity = std::stod(val);
            else if (key == "timestamp") tick.timestamp = val;
        }
        SPDLOG_DEBUG("📥 Parsed tick: {} {} @ {} qty {}", tick.timestamp, tick.symbol, tick.price, tick.quantity);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ Error parsing trade: {}", e.what());
    }
    return tick;
}

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    SPDLOG_INFO("🚀 Starting kline-service");

    startHealthCheckServer(8081);
    ConfigLoader::loadEnv();

    std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
    int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));
    std::string streamKey = ConfigLoader::getEnv("TRADE_STREAM", "TRADE_STREAM:BTC-USDT");
    std::string publishChannel = ConfigLoader::getEnv("PUBLISH_CHANNEL", "ohlcv-stream:BTC-USDT");
    int flushInterval = std::stoi(ConfigLoader::getEnv("FLUSH_INTERVAL_SECONDS", "10"));

    redisContext* redis = redisConnect(redisHost.c_str(), redisPort);
    if (!redis || redis->err) {
        SPDLOG_CRITICAL("❌ Redis connection error: {}", (redis ? redis->errstr : "Unknown"));
        return 1;
    }
    SPDLOG_INFO("✅ Connected to Redis at {}:{}", redisHost, redisPort);

    boost::asio::io_context io;
    global_io = &io;  // 🔗 make io accessible to signal handler

    auto aggregator = std::make_shared<CandleAggregator>();

    KlineScheduler scheduler(io, aggregator, [&](const std::vector<Candle>& candles) {
        auto flushStart = std::chrono::steady_clock::now();

        auto aggStart = std::chrono::steady_clock::now();
        auto aggEnd = std::chrono::steady_clock::now();
        double aggTime = std::chrono::duration<double>(aggEnd - aggStart).count();

        auto publishStart = std::chrono::steady_clock::now();
        for (const auto& c : candles) {
            try {
                std::ostringstream oss;
                oss << R"({"symbol":")" << c.symbol
                    << R"(","interval":")" << c.interval
                    << R"(","startTime":")" << c.startTime
                    << R"(","open":)" << c.open
                    << R"(,"high":)" << c.high
                    << R"(,"low":)" << c.low
                    << R"(,"close":)" << c.close
                    << R"(,"volume":)" << c.volume
                    << R"(,"tradeCount":)" << c.tradeCount
                    << "}";

                redisCommand(redis, "PUBLISH %s %s", publishChannel.c_str(), oss.str().c_str());
                SPDLOG_DEBUG("📤 Published to Redis: {}", oss.str());
            } catch (const std::exception& e) {
                SPDLOG_ERROR("❌ Publish error: {}", e.what());
            }
        }
        auto publishEnd = std::chrono::steady_clock::now();
        double publishTime = std::chrono::duration<double>(publishEnd - publishStart).count();

        auto flushEnd = std::chrono::steady_clock::now();
        double totalFlushTime = std::chrono::duration<double>(flushEnd - flushStart).count();

        flushCount.store(static_cast<int>(candles.size()));
        lastFlushDuration.store(totalFlushTime);

        SPDLOG_INFO("📊 Flush metrics — candles: {}, aggregation: {:.4f}s, publish: {:.4f}s, total: {:.4f}s",
                    flushCount.load(), aggTime, publishTime, totalFlushTime);
    }, flushInterval);

    scheduler.start();

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::thread redisListener([&]() {
        std::string lastId = "$";
        while (running) {
            try {
                redisReply* reply = (redisReply*)redisCommand(redis, "XREAD BLOCK 1000 COUNT 1 STREAMS %s %s",
                                                              streamKey.c_str(), lastId.c_str());
                if (!reply) {
                    SPDLOG_WARN("⚠️ Null Redis reply.");
                    continue;
                }

                if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
                    redisReply* streamData = reply->element[0]->element[1];
                    for (size_t i = 0; i < streamData->elements; ++i) {
                        redisReply* entry = streamData->element[i];
                        std::string entryId = entry->element[0]->str;
                        redisReply* kv = entry->element[1];
                        SPDLOG_DEBUG("🧩 Received trade entry: ID = {}", entryId);
                        TradeTick tick = parseTrade(kv);
                        aggregator->addTrade(tick);
                        lastId = entryId;
                    }
                }

                freeReplyObject(reply);
            } catch (const std::exception& e) {
                SPDLOG_ERROR("❌ Redis listener error: {}", e.what());
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        SPDLOG_INFO("🧵 Redis listener thread exiting.");
    });

    io.run();
    redisListener.join();
    redisFree(redis);
    SPDLOG_INFO("✅ kline-service exited cleanly.");
    return 0;
}

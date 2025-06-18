#include "rms/rms_engine.hpp"
#include "rms/user_state.hpp"
#include "rms/metrics.hpp"
#include "config_loader.hpp"
#include "margin_listener.hpp"
#include "redis_utils.hpp"
#include "redis_circuit.hpp"
#include "thread_safe_map.hpp"

#include <boost/asio.hpp>
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

using boost::asio::steady_timer;
using namespace std::chrono_literals;

// Globals
static boost::asio::io_context io;
static boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work = boost::asio::make_work_guard(io);
static std::vector<std::thread> threadPool;
static std::atomic<bool> running = true;

std::vector<std::string> users = {"user-001", "user-002", "user-003"};
ThreadSafeMap<std::string, std::shared_ptr<rms::RMSEngine>> rmsMap;
std::unordered_map<std::string, double> ltpCache;
redisContext* redisCtx = nullptr;
RedisCircuitBreaker breaker(3, 15);

// Graceful shutdown handler
void handleSignal(int signal) {
    SPDLOG_WARN("🛑 Signal received. Initiating graceful shutdown...");
    running = false;
    io.stop();  // stop all timers
}

// Batched RMS loop
void scheduleBatchRMSLoop() {
    auto timer = std::make_shared<steady_timer>(io, 1s);
    timer->async_wait([timer](const boost::system::error_code& ec) {
        if (ec || !running) return;

        try {
            if (breaker.isOpen()) {
                SPDLOG_WARN("⏸️ Skipping RMS: Redis circuit open.");
            } else {
                if (!redisCtx || redisCtx->err) {
                    redisCtx = safeRedisConnect("127.0.0.1", 6379);
                    if (!redisCtx || redisCtx->err) {
                        breaker.recordFailure();
                        SPDLOG_ERROR("❌ Redis reconnect failed in RMS loop: {}", redisCtx ? redisCtx->errstr : "null");
                        timer->expires_after(1s);
                        scheduleBatchRMSLoop();
                        return;
                    }
                }

                auto ltpOpt = getLatestLTP("BTC-USDT", redisCtx);
                if (!ltpOpt.has_value()) {
                    breaker.recordFailure();
                    SPDLOG_WARN("⚠️ No LTP — fallback margin check will be used.");
                } else {
                    breaker.recordSuccess();
                    double newLtp = ltpOpt.value();
                    double& cached = ltpCache["BTC-USDT"];

                    if (cached != newLtp) {
                        SPDLOG_INFO("🔁 BTC LTP updated: {}", newLtp);
                        cached = newLtp;

                        rmsMap.forEach([&newLtp](const std::string& userId, const std::shared_ptr<rms::RMSEngine>& engine) {
                            boost::asio::post(io, [engine, newLtp]() {
                                try {
                                    engine->onPriceUpdate(newLtp);
                                    engine->syncMargin();
                                } catch (const std::exception& ex) {
                                    SPDLOG_ERROR("🧨 RMS update failed for user {}: {}", engine->getUserId(), ex.what());
                                }
                            });
                        });
                    }
                }
            }
        } catch (const std::exception& ex) {
            SPDLOG_ERROR("❌ Batch RMS loop error: {}", ex.what());
        }

        if (running) {
            timer->expires_after(1s);
            scheduleBatchRMSLoop();  // Reschedule
        }
    });
}

// Main
int main() {
    try {
        SPDLOG_INFO("🚀 Starting RMS Risk Service...");

        std::signal(SIGINT, handleSignal);

        ConfigLoader::loadEnv(".env");

        double fallbackMarginRatio = std::stod(ConfigLoader::getEnv("DEFAULT_MARGIN_RATIO", "0.05"));
        double fallbackLTP = std::stod(ConfigLoader::getEnv("DEFAULT_LTP", "26000"));

        redisCtx = safeRedisConnect("127.0.0.1", 6379);
        if (!redisCtx || redisCtx->err) {
            SPDLOG_CRITICAL("❌ Failed to connect to Redis. Exiting.");
            return EXIT_FAILURE;
        }

        MarginListener marginListener;
        for (const auto& userId : users) {
            rms::UserState state;
            state.userId = userId;
            state.symbol = "BTC-USDT";
            rmsMap.insert(userId, std::make_shared<rms::RMSEngine>(state));
        }

        scheduleBatchRMSLoop();

        try {
            marginListener.start();
        } catch (const std::exception& e) {
            SPDLOG_ERROR("❌ Exception in MarginListener: {}", e.what());
        } catch (...) {
            SPDLOG_ERROR("❌ Unknown fatal error in MarginListener");
        }

        // Launch threads
        for (int i = 0; i < 4; ++i) {
            threadPool.emplace_back([] {
                try {
                    io.run();
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("🧵 IO thread crashed: {}", e.what());
                }
            });
        }

        // Wait for threads to finish
        for (auto& t : threadPool)
            if (t.joinable()) t.join();

        // Cleanup
        if (redisCtx) {
            redisFree(redisCtx);
            SPDLOG_INFO("🧹 Redis context cleaned up.");
        }

        SPDLOG_INFO("👋 Graceful shutdown complete.");
        return 0;

    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("💥 RMS main failed: {}", ex.what());
        return EXIT_FAILURE;
    }
}

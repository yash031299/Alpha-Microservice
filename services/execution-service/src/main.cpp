#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cstdlib>
#include <fstream>
#include <thread>
#include <chrono>
#include "execution_router.hpp"
#include "logger.hpp"
#include "order.pb.h"
#include <netinet/in.h>  // For sockaddr_in, INADDR_ANY, etc.
#include <sys/socket.h>  // For socket(), bind(), listen(), accept()
#include <unistd.h>      // For close(), read(), write()
#include <hiredis/hiredis.h>  // For redisContext, redisCommand, redisReply
#include <nlohmann/json.hpp>

// Simple healthcheck HTTP server
void startHealthCheckServer(int port) {
    std::thread([port]() {
        SPDLOG_INFO("⚕️  Healthcheck running at http://0.0.0.0:{}/healthz", port);
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(sockfd, (sockaddr*)&addr, sizeof(addr));
        listen(sockfd, 5);

        while (true) {
            int client = accept(sockfd, nullptr, nullptr);
            const char* resp =
                "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK";
            send(client, resp, strlen(resp), 0);
            close(client);
        }
    }).detach();
}

void setupLogging() {
    auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
}

void loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t delimPos = line.find('=');
        if (delimPos != std::string::npos) {
            std::string key = line.substr(0, delimPos);
            std::string val = line.substr(delimPos + 1);
            setenv(key.c_str(), val.c_str(), 1);
        }
    }
}

int main() {
    setupLogging();
    try {
        loadEnv(".env");

        // Start healthcheck on separate port (e.g., 8080)
        startHealthCheckServer(8080);

        std::thread([] {
            try {
                std::string redis_host = std::getenv("REDIS_HOST") ? std::getenv("REDIS_HOST") : "127.0.0.1";
                int redis_port = std::getenv("REDIS_PORT") ? std::stoi(std::getenv("REDIS_PORT")) : 6379;
                std::string queue_name = "QUEUE:ORDER";

                redisContext* redis = redisConnect(redis_host.c_str(), redis_port);
                if (!redis || redis->err) {
                    SPDLOG_ERROR("❌ Failed to connect to Redis: {}", redis ? redis->errstr : "unknown");
                    return;
                }

                SPDLOG_INFO("📥 Listening to Redis queue: {}", queue_name);

                while (true) {
                    redisReply* reply = (redisReply*)redisCommand(redis, "BLPOP %s 0", queue_name.c_str());
                    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements != 2) {
                        SPDLOG_WARN("⚠️ Redis BLPOP returned invalid result.");
                        if (reply) freeReplyObject(reply);
                        continue;
                    }

                    std::string jsonStr = reply->element[1]->str;
                    freeReplyObject(reply);

                    SPDLOG_INFO("📝 Received raw order JSON: {}", jsonStr);
                    ExecutionLogger::log(jsonStr, "REDIS");

                    try {
                        nlohmann::json j = nlohmann::json::parse(jsonStr);
                        order::OrderRequest req;
                        
                        req.set_order_id(j.value("order_id", ""));
                        req.set_user_id(j.at("userId").get<std::string>());
                        req.set_symbol(j.at("symbol").get<std::string>());
                        req.set_price(j.at("price").get<double>());
                        req.set_quantity(j.at("quantity").get<double>());
                        req.set_side(j.at("side").get<std::string>());
                        req.set_ordertype(j.at("orderType").get<std::string>());
                        req.set_ltp(j.value("ltp", 0.0));
                        req.set_stopprice(j.value("stopprice", 0.0));
                        req.set_leverage(j.value("leverage", 10.0));

                        order::OrderResponse resp = ExecutionRouter::route(req);
                        SPDLOG_INFO("🎯 Execution result: {} - {}", resp.status(), resp.message());
                    } catch (const std::exception& e) {
                        SPDLOG_ERROR("🚨 Failed to process Redis order: {}", e.what());
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));  // prevent tight loop
                }

            } catch (const std::exception& e) {
                SPDLOG_CRITICAL("❌ Fatal error in Redis listener: {}", e.what());
            }
        }).detach();
        
        // Prevent main from exiting
        while (true) std::this_thread::sleep_for(std::chrono::hours(24));

    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("❌ Uncaught exception in execution-service: {}", ex.what());
    } catch (...) {
        SPDLOG_CRITICAL("❌ Unknown fatal error in execution-service");
    }
    return 0;
}

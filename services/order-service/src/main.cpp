#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "config_loader.hpp"
#include "controller.hpp"
#include "symbol_client.hpp"
#include "user_client.hpp"
#include "risk_client.hpp"
#include "validator.hpp"
#include "grpc_client.hpp"
#include "redis_publisher.hpp"
#include "metrics.hpp"

void setupLogging() {
    auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
}

int main() {
    try {
        setupLogging();
        ConfigLoader::load();

        // Read env
        const std::string order_port  = ConfigLoader::get("ORDER_SERVICE_PORT");
        const std::string exec_host   = ConfigLoader::get("EXECUTION_SERVICE_HOST");
        const std::string symbol_host = ConfigLoader::get("SYMBOL_SERVICE_HOST");
        const std::string user_host   = ConfigLoader::get("USER_SERVICE_HOST");
        const std::string redis_host  = ConfigLoader::get("REDIS_HOST");
        int redis_port                = std::stoi(ConfigLoader::get("REDIS_PORT"));

        SPDLOG_INFO("✅ Environment loaded");
        SPDLOG_INFO("🔗 Connecting to symbol: {}, user: {}, exec: {}", symbol_host, user_host, exec_host);

        // Setup channels
        auto symbolChannel = grpc::CreateChannel(symbol_host, grpc::InsecureChannelCredentials());
        auto userChannel   = grpc::CreateChannel(user_host, grpc::InsecureChannelCredentials());
        auto execChannel   = grpc::CreateChannel(exec_host, grpc::InsecureChannelCredentials());

        // Setup clients
        auto symbolClient = std::make_shared<SymbolClient>(symbolChannel);
        auto userClient   = std::make_shared<UserClient>(userChannel);
        auto execClient   = std::make_shared<ExecutionClient>(execChannel);
        auto redisPublisher = std::make_shared<RedisPublisher>(redis_host, redis_port, "FAILED_ORDERS");
        auto riskClient = std::make_shared<RiskClient>(redis_host, redis_port);


        // Create validator
        auto validator = std::make_shared<Validator>(symbolClient, userClient);

        // Init Prometheus metrics
        auto metrics = Metrics::init();  // starts on :8081

        // Setup controller
        auto controller = std::make_unique<OrderController>(validator, execClient, redisPublisher, metrics, riskClient);

        // gRPC server
        grpc::ServerBuilder builder;
        builder.AddListeningPort("0.0.0.0:" + order_port, grpc::InsecureServerCredentials());
        builder.RegisterService(controller.get());

        auto server = builder.BuildAndStart();
        SPDLOG_INFO("🚀 order-service started on port {}", order_port);

        server->Wait();
    } catch (const std::exception& ex) {
        SPDLOG_CRITICAL("❌ Startup failed: {}", ex.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("❌ Unknown error occurred during startup.");
        return EXIT_FAILURE;
    }

    return 0;
}

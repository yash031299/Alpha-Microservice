#include "grpc_server.hpp"
#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// Constructor
SymbolSnapshotServiceImpl::SymbolSnapshotServiceImpl(std::shared_ptr<OrderBookManager> manager)
    : manager_(std::move(manager)) {}

grpc::Status SymbolSnapshotServiceImpl::GetL1(grpc::ServerContext*,
                                              const snapshot::SymbolRequest* request,
                                              snapshot::L1SnapshotResponse* response) {
    try {
        const std::string& symbol = request->symbol();
        auto levels = manager_->getTopOfBook(symbol, 1);

        response->set_symbol(symbol);
        if (levels.size() > 0) {
            auto* bid = response->mutable_bid();
            bid->set_price(levels[0].price);
            bid->set_qty(levels[0].quantity);
        }
        if (levels.size() > 1) {
            auto* ask = response->mutable_ask();
            ask->set_price(levels[1].price);
            ask->set_qty(levels[1].quantity);
        }

        // Add timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&t), "%FT%TZ");
        response->set_timestamp(oss.str());

        SPDLOG_INFO("📡 GetL1 served for symbol '{}'", symbol);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("GetL1 failed: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Snapshot error");
    }
}

void runGrpcServer(std::shared_ptr<SymbolSnapshotServiceImpl> service, const std::string& address) {
    try {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(service.get());

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        SPDLOG_INFO("📡 gRPC server listening on {}", address);
        server->Wait();
    } catch (const std::exception& e) {
        SPDLOG_ERROR("gRPC server failed: {}", e.what());
    }
}

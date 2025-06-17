#include "grpc_client.hpp"
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

ExecutionClient::ExecutionClient(std::shared_ptr<grpc::Channel> channel, int maxRetries)
    : stub_(order::OrderService::NewStub(channel)), maxRetries_(maxRetries) {}

order::OrderResponse ExecutionClient::submitOrder(const order::OrderRequest& request) {
    order::OrderResponse response;

    for (int attempt = 1; attempt <= maxRetries_; ++attempt) {
        grpc::ClientContext context;
        grpc::Status status = stub_->SubmitOrder(&context, request, &response);

        if (status.ok()) {
            SPDLOG_INFO("✅ Execution RPC succeeded on attempt {}", attempt);
            return response;
        }

        SPDLOG_WARN("❌ Execution RPC failed (attempt {}): {}", attempt, status.error_message());
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt)); // exponential backoff
    }

    // All retries failed
    response.set_status("REJECTED");
    response.set_message("Execution RPC failed after retries");

    return response;
}

order::CancelResponse ExecutionClient::cancelOrder(const order::CancelRequest& request) {
    order::CancelResponse response;
    for (int attempt = 1; attempt <= maxRetries_; ++attempt) {
        grpc::ClientContext context;
        grpc::Status status = stub_->CancelOrder(&context, request, &response);
        if (status.ok()) return response;

        SPDLOG_WARN("CancelOrder RPC failed (attempt {}): {}", attempt, status.error_message());
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * attempt));
    }

    response.set_status("REJECTED");
    response.set_message("Cancel failed after retries");
    return response;
}

order::ModifyResponse ExecutionClient::modifyOrder(const order::ModifyRequest& request) {
    order::ModifyResponse response;
    for (int attempt = 1; attempt <= maxRetries_; ++attempt) {
        grpc::ClientContext context;
        grpc::Status status = stub_->ModifyOrder(&context, request, &response);
        if (status.ok()) return response;

        SPDLOG_WARN("ModifyOrder RPC failed (attempt {}): {}", attempt, status.error_message());
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * attempt));
    }

    response.set_status("REJECTED");
    response.set_message("Modify failed after retries");
    return response;
}

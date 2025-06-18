#include "match_engine_client.hpp"
#include <spdlog/spdlog.h>

MatchEngineClient::MatchEngineClient(const std::string& target) {
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub_ = match::MatchEngine::NewStub(channel);
}

bool MatchEngineClient::submitOrder(const match::OrderRequest& request, match::TradeResponse& response) {
    grpc::ClientContext ctx;
    grpc::Status status = stub_->SubmitOrder(&ctx, request, &response);

    if (!status.ok()) {
        SPDLOG_ERROR("❌ MatchEngine gRPC failed: {}", status.error_message());
        return false;
    }

    SPDLOG_INFO("✅ MatchEngine gRPC returned {} trades", response.trades_size());
    return true;
}

#include "rms_client.hpp"
#include <spdlog/spdlog.h>

RMSClient::RMSClient(const std::string& target) {
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub_ = pnl::PnLService::NewStub(channel);
}

double RMSClient::getEquity(const std::string& user_id, bool& success) {
    pnl::PnLRequest req;
    req.set_user_id(user_id);

    pnl::PnLResponse res;
    grpc::ClientContext ctx;

    grpc::Status status = stub_->GetEquity(&ctx, req, &res);

    if (!status.ok()) {
        SPDLOG_ERROR("❌ RMS GetEquity gRPC failed: {}", status.error_message());
        success = false;
        return 0.0;
    }

    success = true;
    SPDLOG_INFO("✅ RMS returned equity {:.2f} for user {}", res.equity(), user_id);
    return res.equity();
}

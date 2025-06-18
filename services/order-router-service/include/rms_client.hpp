#pragma once

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "pnl.grpc.pb.h"

class RMSClient {
public:
    explicit RMSClient(const std::string& target);

    // Get user's equity from RMS
    double getEquity(const std::string& user_id, bool& success);

private:
    std::unique_ptr<pnl::PnLService::Stub> stub_;
};

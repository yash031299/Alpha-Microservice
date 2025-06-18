#pragma once

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "match.grpc.pb.h"

class MatchEngineClient {
public:
    explicit MatchEngineClient(const std::string& target);

    // Submit a LIMIT order
    bool submitOrder(const match::OrderRequest& request, match::TradeResponse& response);

private:
    std::unique_ptr<match::MatchEngine::Stub> stub_;
};

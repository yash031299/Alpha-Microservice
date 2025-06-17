#pragma once

#include <memory>
#include <string>
#include <optional>
#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"

class ExecutionClient {
public:
    explicit ExecutionClient(std::shared_ptr<grpc::Channel> channel, int maxRetries = 3);
    order::OrderResponse submitOrder(const order::OrderRequest& request);
    order::CancelResponse cancelOrder(const order::CancelRequest& request);
    order::ModifyResponse modifyOrder(const order::ModifyRequest& request);

private:
    std::unique_ptr<order::OrderService::Stub> stub_;
    int maxRetries_;
};

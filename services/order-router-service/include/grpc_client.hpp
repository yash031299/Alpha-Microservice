#pragma once
#include <string>
#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"

class ExecutionGRPCClient {
public:
    explicit ExecutionGRPCClient(const std::string& target);
    bool sendOrderFromJson(const std::string& json);

private:
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<order::OrderService::Stub> stub;
};

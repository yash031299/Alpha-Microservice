#pragma once

#include <memory>
#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"
#include "validator.hpp"
#include "grpc_client.hpp"
#include "redis_publisher.hpp"
#include "risk_client.hpp"
#include "metrics.hpp"

class OrderController final : public order::OrderService::Service {
public:
    OrderController(std::shared_ptr<Validator> validator,
                    std::shared_ptr<ExecutionClient> exec_client,
                    std::shared_ptr<RedisPublisher> redis_publisher,
                    std::shared_ptr<Metrics> metrics,
                    std::shared_ptr<RiskClient> risk_client);

    // gRPC method overrides
    grpc::Status SubmitOrder(grpc::ServerContext*,
                             const order::OrderRequest*,
                             order::OrderResponse*) override;

    grpc::Status CancelOrder(grpc::ServerContext*,
                             const order::CancelRequest*,
                             order::CancelResponse*) override;

    grpc::Status ModifyOrder(grpc::ServerContext*,
                             const order::ModifyRequest*,
                             order::ModifyResponse*) override;

    grpc::Status CheckHealth(grpc::ServerContext*,
                             const order::HealthRequest*,
                             order::HealthResponse*) override;

private:
    std::shared_ptr<Validator> validator_;
    std::shared_ptr<ExecutionClient> exec_client_;
    std::shared_ptr<RedisPublisher> redis_publisher_;
    std::shared_ptr<Metrics> metrics_;
    std::shared_ptr<RiskClient> riskClient_;
};

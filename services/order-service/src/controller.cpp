#include "controller.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <prometheus/counter.h>
#include <prometheus/histogram.h>


OrderController::OrderController(std::shared_ptr<Validator> validator,
                                 std::shared_ptr<ExecutionClient> exec_client,
                                 std::shared_ptr<RedisPublisher> redis_publisher,
                                 std::shared_ptr<Metrics> metrics,
                                 std::shared_ptr<RiskClient> risk_client)
    : validator_(std::move(validator)),
      exec_client_(std::move(exec_client)),
      redis_publisher_(std::move(redis_publisher)),
      metrics_(std::move(metrics)),
      riskClient_(std::move(risk_client)) {}


grpc::Status OrderController::SubmitOrder(grpc::ServerContext*,
                                          const order::OrderRequest* request,
                                          order::OrderResponse* response) {
    try {
        metrics_->totalOrders->Increment();
        auto start = std::chrono::steady_clock::now();

        if (!validator_->validateOrder(*request)) {
            metrics_->rejectedOrders->Increment();
            response->set_status("REJECTED");
            response->set_message("Validation failed");
            return grpc::Status::OK;
        }

        if (!riskClient_->checkMargin(request->user_id(), nlohmann::json{
            {"userId", request->user_id()},
                {"symbol", request->symbol()},
                {"price", request->price()},
                {"quantity", request->quantity()},
                {"side", request->side()},
                {"orderType", request->ordertype()},
            
            })) {
            metrics_->rejectedOrders->Increment();
            response->set_status("REJECTED");
            response->set_message("Rejected by risk engine");
            return grpc::Status::OK;
        }


        auto result = exec_client_->submitOrder(*request);
        *response = result;

        auto latency = std::chrono::steady_clock::now() - start;
        metrics_->orderLatency->Observe(
            std::chrono::duration<double, std::milli>(latency).count());

        if (result.status() == "REJECTED") {
            metrics_->rejectedOrders->Increment();
            metrics_->redisFallbacks->Increment();
            redis_publisher_->enqueue(request->SerializeAsString());
        }

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Unhandled exception in SubmitOrder: {}", e.what());
        response->set_status("ERROR");
        response->set_message("Internal server error");
        return grpc::Status(grpc::StatusCode::INTERNAL, "SubmitOrder failed");
    }
}

grpc::Status OrderController::CancelOrder(grpc::ServerContext*,
                                          const order::CancelRequest* request,
                                          order::CancelResponse* response) {
    try {
        auto result = exec_client_->cancelOrder(*request);
        *response = result;
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("CancelOrder exception: {}", e.what());
        response->set_status("ERROR");
        response->set_message("Internal server error");
        return grpc::Status(grpc::StatusCode::INTERNAL, "CancelOrder failed");
    }
}

grpc::Status OrderController::ModifyOrder(grpc::ServerContext*,
                                          const order::ModifyRequest* request,
                                          order::ModifyResponse* response) {
    try {
        auto result = exec_client_->modifyOrder(*request);
        *response = result;
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("ModifyOrder exception: {}", e.what());
        response->set_status("ERROR");
        response->set_message("Internal server error");
        return grpc::Status(grpc::StatusCode::INTERNAL, "ModifyOrder failed");
    }
}

grpc::Status OrderController::CheckHealth(grpc::ServerContext*,
                                          const order::HealthRequest*,
                                          order::HealthResponse* response) {
    response->set_status("SERVING");
    return grpc::Status::OK;
}

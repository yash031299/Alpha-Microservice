#include "doctest/doctest.h"
#include "controller.hpp"
#include "order.pb.h"

// Fake Validator
class FakeValidator : public Validator {
public:
    FakeValidator() : Validator(nullptr, nullptr) {}
    bool validateOrder(const order::OrderRequest&) {
        return true;  // Always validate successfully
    }
};

// Fake ExecutionClient
class FakeExecClient : public ExecutionClient {
public:
    FakeExecClient() : ExecutionClient(nullptr) {}

    order::OrderResponse submitOrder(const order::OrderRequest&)  {
        order::OrderResponse res;
        res.set_status("ACCEPTED");
        res.set_message("Order placed.");
        return res;
    }
};

// Fake RedisPublisher
class FakeRedisPublisher : public RedisPublisher {
public:
    FakeRedisPublisher() : RedisPublisher("localhost", 6379, "dummy") {}
    void enqueue(const std::string& jsonPayload) {
        // Do nothing
    }
};

TEST_CASE("OrderController: handles SubmitOrder") {
    auto validator = std::make_shared<FakeValidator>();
    auto exec = std::make_shared<FakeExecClient>();
    auto redis = std::make_shared<FakeRedisPublisher>();
    auto metrics = Metrics::init();
    auto riskClient = std::make_shared<RiskClient>("127.0.0.1", 6379);

    OrderController controller(validator, exec, redis, metrics, riskClient);


    order::OrderRequest request;
    request.set_user_id("u1");
    request.set_symbol("BTC");
    request.set_price(10000);
    request.set_quantity(0.1);
    request.set_ordertype("LIMIT");
    request.set_side("BUY");

    order::OrderResponse response;
    grpc::ServerContext context;

    auto status = controller.SubmitOrder(&context, &request, &response);

    CHECK(status.ok());
    CHECK(response.status() == "ACCEPTED");
}

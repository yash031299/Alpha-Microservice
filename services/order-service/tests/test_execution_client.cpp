#include "doctest/doctest.h"
#include "grpc_client.hpp"

TEST_CASE("ExecutionClient: handles gRPC failure") {
    ExecutionClient client(grpc::CreateChannel("bad-host:1234", grpc::InsecureChannelCredentials()));
    
    order::OrderRequest req;
    req.set_symbol("BTC");
    req.set_user_id("user123");
    req.set_price(10000);
    req.set_quantity(0.5);

    auto res = client.submitOrder(req);

    CHECK(res.status() == "REJECTED");
}

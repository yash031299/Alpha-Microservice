#include "doctest/doctest.h"
#include "validator.hpp"
#include "symbol_client.hpp"
#include "user_client.hpp"
#include <memory>

TEST_CASE("Validator: reject non-existent symbol") {
    auto fakeSymbolClient = std::make_shared<SymbolClient>(grpc::CreateChannel("invalid", grpc::InsecureChannelCredentials()));
    auto fakeUserClient = std::make_shared<UserClient>(grpc::CreateChannel("invalid", grpc::InsecureChannelCredentials()));
    Validator validator(fakeSymbolClient, fakeUserClient);

    order::OrderRequest req;
    req.set_symbol("FAKE");
    req.set_user_id("user123");
    req.set_price(10000);
    req.set_quantity(1);
    req.set_leverage(10);

    CHECK(validator.validateOrder(req) == false);
}

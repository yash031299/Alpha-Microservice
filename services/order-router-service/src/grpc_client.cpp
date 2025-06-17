#include "grpc_client.hpp"
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

ExecutionGRPCClient::ExecutionGRPCClient(const std::string& target) {
    channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub = order::OrderService::NewStub(channel);
}

bool ExecutionGRPCClient::sendOrderFromJson(const std::string& json) {
    rapidjson::Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError()) {
        SPD_ERROR("Invalid JSON format");
        return false;
    }

    order::OrderRequest req;
    req.set_user_id(doc["userId"].GetString());
    req.set_symbol(doc["symbol"].GetString());
    req.set_price(doc["price"].GetDouble());
    req.set_quantity(doc["quantity"].GetDouble());
    req.set_side(doc["side"].GetString());
    req.set_ordertype(doc["ordertype"].GetString());
    req.set_ltp(doc["ltp"].GetDouble());
    req.set_stopprice(doc["stopprice"].GetDouble());

    order::OrderResponse res;
    grpc::ClientContext ctx;
    grpc::Status status = stub->SubmitOrder(&ctx, req, &res);

    if (!status.ok()) {
        SPD_ERROR("gRPC failed: {}", status.error_message());
        return false;
    }

    SPD_INFO("gRPC success: {} - {}", res.status(), res.message());
    return res.status() == "ACCEPTED";
}

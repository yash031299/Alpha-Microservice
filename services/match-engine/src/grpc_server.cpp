#include "grpc_server.hpp"
#include "book.hpp"
#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>
#include "match.grpc.pb.h"
#include "match.pb.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

MatchEngineServiceImpl::MatchEngineServiceImpl(Matcher& matcher, TradePublisher& publisher)
    : matcher_(matcher), publisher_(publisher) {}

Status MatchEngineServiceImpl::SubmitOrder(ServerContext*,
                                           const match::OrderRequest* req,
                                           match::TradeResponse* resp) {
    try {
        Order order;
        order.orderId = req->orderid();
        order.userId = req->userid();
        order.symbol = req->symbol();
        order.price = req->price();
        order.quantity = req->quantity();
        order.isBuy = (req->direction() == "BUY");

        // 🧠 Parse optional fields
        if (req->has_ordertype()) {
            std::string type = req->ordertype();
            if (type == "LIMIT") order.orderType = OrderType::LIMIT;
            else if (type == "MARKET") order.orderType = OrderType::MARKET;
            else if (type == "STOP_LIMIT") order.orderType = OrderType::STOP_LIMIT;
        }

        if (req->has_tif()) {
            std::string tif = req->tif();
            if (tif == "IOC") order.tif = TimeInForce::IOC;
            else if (tif == "FOK") order.tif = TimeInForce::FOK;
            else order.tif = TimeInForce::GTC;
        }

        if (req->has_trigger_price()) {
            order.triggerPrice = req->trigger_price();
        }

        // 🧮 Matching
        auto trades = matcher_.submit(order);
        publisher_.publish(trades);

        for (const auto& t : trades) {
            match::Trade* tr = resp->add_trades();
            tr->set_makerorderid(t.makerOrderId);
            tr->set_takerorderid(t.takerOrderId);
            tr->set_symbol(t.symbol);
            tr->set_price(t.price);
            tr->set_quantity(t.quantity);
            tr->set_timestamp(t.timestamp);
        }

        return Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("💥 SubmitOrder exception: {}", e.what());
        return Status(grpc::INTERNAL, "SubmitOrder failed");
    } catch (...) {
        SPDLOG_ERROR("💥 Unknown SubmitOrder failure");
        return Status(grpc::INTERNAL, "Unknown error");
    }
}


Status MatchEngineServiceImpl::CancelOrder(ServerContext*,
                                           const match::CancelRequest* req,
                                           match::CancelResponse* resp) {
    try {
        bool ok = matcher_.cancel(req->symbol(), req->orderid());
        resp->set_success(ok);
        return Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("CancelOrder error: {}", e.what());
        return Status(grpc::INTERNAL, "Cancel error");
    }
}

grpc::Status MatchEngineServiceImpl::ModifyOrder(grpc::ServerContext* context,
                                                 const match::ModifyRequest* req,
                                                 match::ModifyResponse* resp) {
    try {
        Order newOrder;
        newOrder.orderId  = req->orderid();
        newOrder.userId   = req->userid();
        newOrder.symbol   = req->symbol();
        newOrder.price    = req->new_price();
        newOrder.quantity = req->new_quantity();
        newOrder.isBuy    = (req->direction() == "BUY");

        bool success = matcher_.modify(req->symbol(), newOrder);
        resp->set_success(success);

        if (!success)
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Order not found");

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("ModifyOrder error: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Modify failed");
    }
}


void runGrpcServer(Matcher& matcher, TradePublisher& publisher, const std::string& address) {
    MatchEngineServiceImpl service(matcher, publisher);
    ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    SPDLOG_INFO("🚀 gRPC server listening on {}", address);
    server->Wait();
}

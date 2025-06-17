#ifndef GRPC_SERVER_HPP
#define GRPC_SERVER_HPP

#include "matcher.hpp"
#include "trade_publisher.hpp"
#include "match.grpc.pb.h"
#include "match.pb.h"

class MatchEngineServiceImpl final : public match::MatchEngine::Service {
public:
    MatchEngineServiceImpl(Matcher& matcher, TradePublisher& publisher);

    grpc::Status SubmitOrder(grpc::ServerContext* context,
                             const match::OrderRequest* request,
                             match::TradeResponse* response) override;

    grpc::Status CancelOrder(grpc::ServerContext* context,
                             const match::CancelRequest* request,
                             match::CancelResponse* response) override;
    
    grpc::Status ModifyOrder(grpc::ServerContext* context,
                         const match::ModifyRequest* request,
                         match::ModifyResponse* response) override;



private:
    Matcher& matcher_;
    TradePublisher& publisher_;
};

void runGrpcServer(Matcher& matcher, TradePublisher& publisher, const std::string& address);
void stopGrpcServer(); 

#endif

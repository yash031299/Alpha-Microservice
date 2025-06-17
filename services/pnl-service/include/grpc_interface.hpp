#pragma once

#include <grpcpp/grpcpp.h>
#include "pnl_engine.hpp"
#include "equity_calculator.hpp"
#include "symbol.grpc.pb.h"
#include "pnl.grpc.pb.h"
#include "symbol_client.hpp"
#include <unordered_map>
#include <mutex>
#include <memory>

class PnLServiceImpl final : public pnl::PnLService::Service {
public:
    PnLServiceImpl();

    grpc::Status GetEquity(grpc::ServerContext* context,
                           const pnl::PnLRequest* request,
                           pnl::PnLResponse* response) override;

    grpc::Status CheckLiquidation(grpc::ServerContext* context,
                                  const pnl::PnLRequest* request,
                                  pnl::LiquidationResponse* response) override;

    grpc::Status GetAllPnL(grpc::ServerContext* context,
                           const pnl::PnLRequest* request,
                           pnl::AllPnLResponse* response) override;

    grpc::Status UpdateWallet(grpc::ServerContext* context,
                              const pnl::WalletRequest* request,
                              pnl::WalletResponse* response) override;

    grpc::Status SendTrade(grpc::ServerContext* context,
                           const pnl::TradeUpdate* request,
                           pnl::Empty* response) override;

    std::shared_ptr<EquityCalculator> getOrCreateUser(const std::string& userId);

private:
    std::unordered_map<std::string, std::shared_ptr<EquityCalculator>> users_;
    std::shared_ptr<SymbolServiceClient> symbolClient_;
    std::mutex user_mutex_;
};

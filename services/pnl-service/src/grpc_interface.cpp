#include "grpc_interface.hpp"
#include "config_loader.hpp"
#include <spdlog/spdlog.h>

PnLServiceImpl::PnLServiceImpl() {
    try {
        std::string symbolHost = ConfigLoader::getEnv("SYMBOL_GRPC_HOST", "localhost:6000");
        symbolClient_ = std::make_shared<SymbolServiceClient>(symbolHost);
        SPDLOG_INFO("✅ Connected to Symbol Registry at {}", symbolHost);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ Failed to initialize SymbolServiceClient: {}", e.what());
    }
}

std::shared_ptr<EquityCalculator> PnLServiceImpl::getOrCreateUser(const std::string& userId) {
    std::lock_guard<std::mutex> lock(user_mutex_);
    if (!users_.count(userId)) {
        auto eq = std::make_shared<EquityCalculator>(userId, 1000.0);  // default wallet
        eq->setSymbolService(symbolClient_);
        users_[userId] = eq;
    }
    return users_[userId];
}

grpc::Status PnLServiceImpl::GetEquity(grpc::ServerContext*,
                                       const pnl::PnLRequest* request,
                                       pnl::PnLResponse* response) {
    try {
        auto user = getOrCreateUser(request->user_id());
        response->set_user_id(request->user_id());
        response->set_equity(user->getEquity());
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ GetEquity failed: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to get equity");
    }
}

grpc::Status PnLServiceImpl::CheckLiquidation(grpc::ServerContext*,
                                              const pnl::PnLRequest* request,
                                              pnl::LiquidationResponse* response) {
    try {
        auto user = getOrCreateUser(request->user_id());
        response->set_user_id(request->user_id());
        response->set_should_liquidate(user->isLiquidatable());
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ CheckLiquidation failed: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Liquidation check failed");
    }
}

grpc::Status PnLServiceImpl::GetAllPnL(grpc::ServerContext*,
                                       const pnl::PnLRequest* request,
                                       pnl::AllPnLResponse* response) {
    try {
        auto user = getOrCreateUser(request->user_id());
        auto pnl = user->computePnL();  // ✅ Use public method

        response->set_user_id(request->user_id());
        response->set_unrealized(pnl.unrealized);
        response->set_realized(pnl.realized);
        response->set_equity(pnl.equity);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ GetAllPnL failed: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to get PnL");
    }
}

grpc::Status PnLServiceImpl::UpdateWallet(grpc::ServerContext*,
                                          const pnl::WalletRequest* request,
                                          pnl::WalletResponse* response) {
    try {
        auto user = getOrCreateUser(request->user_id());
        double delta = request->delta();

        user->addToWallet(delta);  // ✅ Use public interface
        response->set_success(true);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ UpdateWallet failed: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to update wallet");
    }
}

grpc::Status PnLServiceImpl::SendTrade(grpc::ServerContext*,
                                       const pnl::TradeUpdate* request,
                                       pnl::Empty* response) {
    try {
        auto user = getOrCreateUser(request->user_id());
        double qty = request->quantity();
        if (request->side() == "SELL") qty *= -1;

        user->updateLTP(request->symbol(), request->price());
        user->updatePosition(request->symbol(), qty, request->price());

        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ SendTrade failed: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to process trade");
    }
}

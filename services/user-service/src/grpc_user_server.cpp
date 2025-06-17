#include "grpc_user_server.hpp"
#include <spdlog/spdlog.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "user.grpc.pb.h"

UserServiceImpl::UserServiceImpl(std::shared_ptr<UserWalletStore> walletStore,
                                 std::shared_ptr<UserMetadataStore> metadataStore)
    : walletStore_(walletStore), metadataStore_(metadataStore) {}

grpc::Status UserServiceImpl::UpdateWallet(grpc::ServerContext* context,
                                          const user::WalletUpdateRequest* request,
                                          user::WalletResponse* response) {
    try {
        std::string userId = request->user_id();
        double delta = request->amount_change();

        walletStore_->applyDelta(userId, delta);
        double newBalance = walletStore_->getBalance(userId);

        response->set_new_balance(newBalance);
        response->set_success(true);
        // TODO: Calculate available_balance and used_margin properly as per your logic
        response->set_available_balance(newBalance);  
        response->set_used_margin(0.0);

        SPDLOG_INFO("UpdateWallet: UserId={}, Delta={}, NewBalance={}", userId, delta, newBalance);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in UpdateWallet: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Internal error");
    } catch (...) {
        SPDLOG_ERROR("Unknown exception in UpdateWallet");
        return grpc::Status(grpc::StatusCode::INTERNAL, "Unknown internal error");
    }
}

grpc::Status UserServiceImpl::GetUserMetadata(grpc::ServerContext* context,
                                              const user::UserIdRequest* request,
                                              user::UserMetadataResponse* response) {
    try {
        std::string userId = request->user_id();
        UserMetadata meta = metadataStore_->getMetadata(userId);

        response->set_user_id(userId);
        response->set_role("user");  // You can extend to support roles if needed
        response->set_kyc_verified(meta.kycVerified);

        SPDLOG_INFO("GetUserMetadata: UserId={}, KYC={}", userId, meta.kycVerified);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in GetUserMetadata: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Internal error");
    } catch (...) {
        SPDLOG_ERROR("Unknown exception in GetUserMetadata");
        return grpc::Status(grpc::StatusCode::INTERNAL, "Unknown internal error");
    }
}

grpc::Status UserServiceImpl::GetWallet(grpc::ServerContext* context,
                                        const user::UserIdRequest* request,
                                        user::WalletResponse* response) {
    try {
        std::string userId = request->user_id();
        double balance = walletStore_->getBalance(userId);

        response->set_new_balance(balance);
        response->set_success(true);
        // TODO: Calculate available_balance and used_margin properly
        response->set_available_balance(balance);
        response->set_used_margin(0.0);

        SPDLOG_INFO("GetWallet: UserId={}, Balance={}", userId, balance);
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Exception in GetWallet: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, "Internal error");
    } catch (...) {
        SPDLOG_ERROR("Unknown exception in GetWallet");
        return grpc::Status(grpc::StatusCode::INTERNAL, "Unknown internal error");
    }
}

void RunGrpcServer(std::shared_ptr<UserWalletStore> walletStore,
                   std::shared_ptr<UserMetadataStore> metadataStore,
                   const std::string& server_address) {
    UserServiceImpl service(walletStore, metadataStore);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    SPDLOG_INFO("gRPC Server listening on {}", server_address);

    server->Wait();
}

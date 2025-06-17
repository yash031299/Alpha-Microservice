#ifndef GRPC_USER_SERVER_HPP
#define GRPC_USER_SERVER_HPP

#include <grpcpp/grpcpp.h>
#include "user_wallet_store.hpp"
#include "user_metadata_store.hpp"
#include <memory>
#include <string>

#include "user.grpc.pb.h"  // Your generated proto header

class UserServiceImpl final : public user::UserService::Service {
public:
    UserServiceImpl(std::shared_ptr<UserWalletStore> walletStore,
                    std::shared_ptr<UserMetadataStore> metadataStore);

    grpc::Status UpdateWallet(grpc::ServerContext* context,
                              const user::WalletUpdateRequest* request,
                              user::WalletResponse* response) override;

    grpc::Status GetUserMetadata(grpc::ServerContext* context,
                                 const user::UserIdRequest* request,
                                 user::UserMetadataResponse* response) override;

    grpc::Status GetWallet(grpc::ServerContext* context,
                           const user::UserIdRequest* request,
                           user::WalletResponse* response) override;

private:
    std::shared_ptr<UserWalletStore> walletStore_;
    std::shared_ptr<UserMetadataStore> metadataStore_;
};

void RunGrpcServer(std::shared_ptr<UserWalletStore> walletStore,
                   std::shared_ptr<UserMetadataStore> metadataStore,
                   const std::string& server_address);

#endif // GRPC_USER_SERVER_HPP

#include "grpc_user_server.hpp"
#include <spdlog/spdlog.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "user.grpc.pb.h"
#include "redis_safe_client.hpp"

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

        double usedMargin = 0.0;
        auto redis = walletStore_->getRedis();
        if (redis && redis->isConnected()) {
            std::string key = "USER:" + userId;
            redisReply* reply = redis->command("HGETALL %s", key.c_str());
            if (reply && reply->type == REDIS_REPLY_ARRAY) {
                for (size_t i = 0; i < reply->elements; i += 2) {
                    std::string field = reply->element[i]->str;
                    if (field.rfind("pos:", 0) == 0) {
                        std::string val = reply->element[i + 1]->str;
                        std::istringstream ss(val);
                        std::string token;
                        std::vector<std::string> tokens;
                        while (std::getline(ss, token, ',')) tokens.push_back(token);
                        if (tokens.size() >= 4) {
                            double notional = std::stod(tokens[3]);
                            usedMargin += notional * 0.05;
                        }
                    }
                }
            }
            if (reply) freeReplyObject(reply);
        }
        auto available = newBalance - usedMargin;
        response->set_new_balance(newBalance);
        response->set_success(true);
        response->set_available_balance(available);
        response->set_used_margin(usedMargin);


        // 🔁 Trigger risk check for this user
        if (redis && redis->isConnected()) {
            redisReply* pushResult = redis->command("LPUSH QUEUE:MARGIN:CHECK %s", userId.c_str());
            if (pushResult) freeReplyObject(pushResult);
            SPDLOG_INFO("📤 Emitted margin check trigger for user {}", userId);
        } else {
            SPDLOG_WARN("⚠️ Could not emit margin check. Redis not connected");
        }

        SPDLOG_INFO("✅ UpdateWallet: UserId={}, Delta={}, NewBalance={}", userId, delta, newBalance);
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
        double wallet = walletStore_->getBalance(userId);

        double usedMargin = 0.0;
        auto redis = walletStore_->getRedis();
        if (redis && redis->isConnected()) {
            std::string key = "USER:" + userId;
            redisReply* reply = redis->command("HGETALL %s", key.c_str());
            if (reply && reply->type == REDIS_REPLY_ARRAY) {
                for (size_t i = 0; i < reply->elements; i += 2) {
                    std::string field = reply->element[i]->str;
                    if (field.rfind("pos:", 0) == 0) {
                        std::string val = reply->element[i + 1]->str;
                        std::istringstream ss(val);
                        std::string token;
                        std::vector<std::string> tokens;
                        while (std::getline(ss, token, ',')) tokens.push_back(token);
                        if (tokens.size() >= 4) {
                            double notional = std::stod(tokens[3]);
                            usedMargin += notional * 0.05;  // Initial margin rate
                        }
                    }
                }
            }
            if (reply) freeReplyObject(reply);
        }

        double available = wallet - usedMargin;

        response->set_new_balance(wallet);
        response->set_success(true);
        response->set_available_balance(available);
        response->set_used_margin(usedMargin);

        SPDLOG_INFO("📊 GetWallet: user={}, wallet={:.2f}, used_margin={:.2f}, available={:.2f}",
                    userId, wallet, usedMargin, available);
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

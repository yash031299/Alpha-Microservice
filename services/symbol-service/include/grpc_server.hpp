#ifndef GRPC_SNAPSHOT_SERVER_HPP
#define GRPC_SNAPSHOT_SERVER_HPP

#include "orderbook_manager.hpp"
#include <snapshot.grpc.pb.h>
#include <memory>

class SymbolSnapshotServiceImpl final : public snapshot::SymbolSnapshotService::Service {
public:
    SymbolSnapshotServiceImpl(std::shared_ptr<OrderBookManager> manager);
    grpc::Status GetL1(grpc::ServerContext* context,
                       const snapshot::SymbolRequest* request,
                       snapshot::L1SnapshotResponse* response) override;

private:
    std::shared_ptr<OrderBookManager> manager_;
};

void runGrpcServer(std::shared_ptr<SymbolSnapshotServiceImpl> service, const std::string& address);

#endif // GRPC_SNAPSHOT_SERVER_HPP

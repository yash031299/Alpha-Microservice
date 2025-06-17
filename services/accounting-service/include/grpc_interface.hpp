#ifndef ACCOUNTING_GRPC_INTERFACE_HPP
#define ACCOUNTING_GRPC_INTERFACE_HPP

#include "ledger.hpp"
#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>

class AccountingGRPCServer {
public:
    explicit AccountingGRPCServer(std::shared_ptr<Ledger> ledger);
    void run(const std::string& address);
    void shutdown();  // ✅ Enables graceful shutdown

private:
    std::shared_ptr<Ledger> ledger_;
    std::unique_ptr<grpc::Server> server_;  // ✅ Needed for shutdown support
};

#endif  // ACCOUNTING_GRPC_INTERFACE_HPP

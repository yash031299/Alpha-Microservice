#ifndef SYMBOL_CLIENT_HPP
#define SYMBOL_CLIENT_HPP

#include "symbol.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <string>


class SymbolServiceClient {
public:
    explicit SymbolServiceClient(const std::string& server_address);

    symbol::SymbolResponse getSymbolMetadata(const std::string& symbol);

private:
    std::unique_ptr<symbol::SymbolService::Stub> stub_;
};

#endif // SYMBOL_CLIENT_HPP

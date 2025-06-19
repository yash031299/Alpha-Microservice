#pragma once
#include <memory>
#include <string>
#include <optional>
#include "symbol.grpc.pb.h"
#include "symbol.pb.h"
#include <grpcpp/grpcpp.h>

struct SymbolInfo {
    std::string symbol;
    double tick_size;
    int precision;
    double max_leverage;
    double initial_margin;
    double maintenance_margin;
};

class SymbolRegistryClient {
public:
    explicit SymbolRegistryClient(const std::string& server_address);

    std::optional<SymbolInfo> getSymbolInfo(const std::string& symbol);

private:
    std::unique_ptr<SymbolRegistry::Stub> stub_;
};

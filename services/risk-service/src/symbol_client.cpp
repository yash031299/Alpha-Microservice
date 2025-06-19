#include "symbol_client.hpp"
#include <grpcpp/create_channel.h>
#include <spdlog/spdlog.h>


SymbolRegistryClient::SymbolRegistryClient(const std::string& server_address) {
    stub_ = SymbolRegistry::NewStub(
        grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
}

std::optional<SymbolInfo> SymbolRegistryClient::getSymbolInfo(const std::string& symbol) {
    SymbolRequest request;
    request.set_symbol(symbol);

    SymbolMetadata response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetSymbolMetadata(&context, request, &response);

    if (!status.ok()) {
        SPDLOG_ERROR("❌ gRPC failed for GetSymbolInfo: {}", status.error_message());
        return std::nullopt;
    }

    SymbolInfo info;
    info.symbol = response.symbol();
    info.tick_size = response.tick_size();
    info.precision = response.precision();
    info.max_leverage = response.max_leverage();
    info.initial_margin = response.initial_margin();
    info.maintenance_margin = response.maintenance_margin();
    return info;
}

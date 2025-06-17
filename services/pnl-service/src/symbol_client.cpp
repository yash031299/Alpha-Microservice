#include "symbol_client.hpp"
#include <spdlog/spdlog.h>

SymbolServiceClient::SymbolServiceClient(const std::string& server_address) {
    stub_ = symbol::SymbolService::NewStub(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));
}

symbol::SymbolResponse SymbolServiceClient::getSymbolMetadata(const std::string& symbol) {
    symbol::SymbolRequest request;
    request.set_symbol(symbol);

    symbol::SymbolResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->GetSymbol(&context, request, &response);

    if (!status.ok()) {
        throw std::runtime_error("Symbol gRPC failed: " + status.error_message());
    }

    if (!response.found()) {
        throw std::runtime_error("Symbol not found in registry: " + symbol);
    }

    return response;
}

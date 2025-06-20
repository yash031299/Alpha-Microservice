// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: log_event.proto

#include "log_event.pb.h"
#include "log_event.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
#include <grpcpp/ports_def.inc>

static const char* LogEventService_method_names[] = {
  "/LogEventService/LogTrade",
  "/LogEventService/LogOrder",
  "/LogEventService/LogFunding",
  "/LogEventService/LogLiquidation",
};

std::unique_ptr< LogEventService::Stub> LogEventService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< LogEventService::Stub> stub(new LogEventService::Stub(channel, options));
  return stub;
}

LogEventService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_LogTrade_(LogEventService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_LogOrder_(LogEventService_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_LogFunding_(LogEventService_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_LogLiquidation_(LogEventService_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status LogEventService::Stub::LogTrade(::grpc::ClientContext* context, const ::TradeEvent& request, ::LogAck* response) {
  return ::grpc::internal::BlockingUnaryCall< ::TradeEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_LogTrade_, context, request, response);
}

void LogEventService::Stub::async::LogTrade(::grpc::ClientContext* context, const ::TradeEvent* request, ::LogAck* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::TradeEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogTrade_, context, request, response, std::move(f));
}

void LogEventService::Stub::async::LogTrade(::grpc::ClientContext* context, const ::TradeEvent* request, ::LogAck* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogTrade_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::PrepareAsyncLogTradeRaw(::grpc::ClientContext* context, const ::TradeEvent& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::LogAck, ::TradeEvent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_LogTrade_, context, request);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::AsyncLogTradeRaw(::grpc::ClientContext* context, const ::TradeEvent& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncLogTradeRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status LogEventService::Stub::LogOrder(::grpc::ClientContext* context, const ::OrderEvent& request, ::LogAck* response) {
  return ::grpc::internal::BlockingUnaryCall< ::OrderEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_LogOrder_, context, request, response);
}

void LogEventService::Stub::async::LogOrder(::grpc::ClientContext* context, const ::OrderEvent* request, ::LogAck* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::OrderEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogOrder_, context, request, response, std::move(f));
}

void LogEventService::Stub::async::LogOrder(::grpc::ClientContext* context, const ::OrderEvent* request, ::LogAck* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogOrder_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::PrepareAsyncLogOrderRaw(::grpc::ClientContext* context, const ::OrderEvent& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::LogAck, ::OrderEvent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_LogOrder_, context, request);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::AsyncLogOrderRaw(::grpc::ClientContext* context, const ::OrderEvent& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncLogOrderRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status LogEventService::Stub::LogFunding(::grpc::ClientContext* context, const ::FundingEvent& request, ::LogAck* response) {
  return ::grpc::internal::BlockingUnaryCall< ::FundingEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_LogFunding_, context, request, response);
}

void LogEventService::Stub::async::LogFunding(::grpc::ClientContext* context, const ::FundingEvent* request, ::LogAck* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::FundingEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogFunding_, context, request, response, std::move(f));
}

void LogEventService::Stub::async::LogFunding(::grpc::ClientContext* context, const ::FundingEvent* request, ::LogAck* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogFunding_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::PrepareAsyncLogFundingRaw(::grpc::ClientContext* context, const ::FundingEvent& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::LogAck, ::FundingEvent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_LogFunding_, context, request);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::AsyncLogFundingRaw(::grpc::ClientContext* context, const ::FundingEvent& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncLogFundingRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status LogEventService::Stub::LogLiquidation(::grpc::ClientContext* context, const ::LiquidationEvent& request, ::LogAck* response) {
  return ::grpc::internal::BlockingUnaryCall< ::LiquidationEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_LogLiquidation_, context, request, response);
}

void LogEventService::Stub::async::LogLiquidation(::grpc::ClientContext* context, const ::LiquidationEvent* request, ::LogAck* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::LiquidationEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogLiquidation_, context, request, response, std::move(f));
}

void LogEventService::Stub::async::LogLiquidation(::grpc::ClientContext* context, const ::LiquidationEvent* request, ::LogAck* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_LogLiquidation_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::PrepareAsyncLogLiquidationRaw(::grpc::ClientContext* context, const ::LiquidationEvent& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::LogAck, ::LiquidationEvent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_LogLiquidation_, context, request);
}

::grpc::ClientAsyncResponseReader< ::LogAck>* LogEventService::Stub::AsyncLogLiquidationRaw(::grpc::ClientContext* context, const ::LiquidationEvent& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncLogLiquidationRaw(context, request, cq);
  result->StartCall();
  return result;
}

LogEventService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LogEventService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LogEventService::Service, ::TradeEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](LogEventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::TradeEvent* req,
             ::LogAck* resp) {
               return service->LogTrade(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LogEventService_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LogEventService::Service, ::OrderEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](LogEventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::OrderEvent* req,
             ::LogAck* resp) {
               return service->LogOrder(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LogEventService_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LogEventService::Service, ::FundingEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](LogEventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::FundingEvent* req,
             ::LogAck* resp) {
               return service->LogFunding(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LogEventService_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LogEventService::Service, ::LiquidationEvent, ::LogAck, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](LogEventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::LiquidationEvent* req,
             ::LogAck* resp) {
               return service->LogLiquidation(ctx, req, resp);
             }, this)));
}

LogEventService::Service::~Service() {
}

::grpc::Status LogEventService::Service::LogTrade(::grpc::ServerContext* context, const ::TradeEvent* request, ::LogAck* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status LogEventService::Service::LogOrder(::grpc::ServerContext* context, const ::OrderEvent* request, ::LogAck* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status LogEventService::Service::LogFunding(::grpc::ServerContext* context, const ::FundingEvent* request, ::LogAck* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status LogEventService::Service::LogLiquidation(::grpc::ServerContext* context, const ::LiquidationEvent* request, ::LogAck* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}



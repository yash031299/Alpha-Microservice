#ifndef REPORT_EXPORT_SERVER_HPP
#define REPORT_EXPORT_SERVER_HPP

#include "report_export.pb.h"
#include "report_export.grpc.pb.h"
#include "report_generator.hpp"
#include <memory>
#include <grpcpp/grpcpp.h>

class ReportExportServiceImpl final : public report::ReportExportService::Service {
public:
    explicit ReportExportServiceImpl(std::shared_ptr<ReportGenerator> generator);
    grpc::Status ExportUserReport(grpc::ServerContext* context,
                                  const report::ExportRequest* request,
                                  report::ExportResponse* response) override;

private:
    std::shared_ptr<ReportGenerator> generator_;
};

void runReportExportServer(std::shared_ptr<ReportGenerator> generator, const std::string& address);

#endif // REPORT_EXPORT_SERVER_HPP

#include "report_export_server.hpp"
#include "config_loader.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

ReportExportServiceImpl::ReportExportServiceImpl(std::shared_ptr<ReportGenerator> generator)
    : generator_(std::move(generator)) {}

Status ReportExportServiceImpl::ExportUserReport(ServerContext*,
                                                 const report::ExportRequest* request,
                                                 report::ExportResponse* response) {
    try {
        SPDLOG_INFO("📥 Received export request: user_id={}, type={}, format={}",
                    request->user_id(), request->type(), request->format());

        // Parse JSON string to JSON object
        nlohmann::json data = nlohmann::json::parse(request->json_data());

        bool success = false;
        std::string exportPath = ConfigLoader::get("EXPORT_OUTPUT_PATH", "data/exports");

        if (request->format() == "csv") {
            success = generator_->generateCSVReport(request->user_id(), data, request->type());
        } else if (request->format() == "pdf") {
            success = generator_->generatePDFReport(request->user_id(), data, request->type());
        } else {
            response->set_success(false);
            response->set_message("Unsupported format. Use 'csv' or 'pdf'");
            return Status::OK;
        }

        response->set_success(success);
        response->set_message(success ? "Report generated successfully." : "Report generation failed.");

        // Find latest matching file
        if (success) {
            std::string latest;
            std::string pattern = request->user_id() + "_" + request->type();
            std::string extension = "." + request->format();

            for (const auto& entry : std::filesystem::directory_iterator(exportPath)) {
                if (entry.path().string().find(pattern) != std::string::npos &&
                    entry.path().extension() == extension) {
                    latest = entry.path().string();  // will take last match
                }
            }

            response->set_file_path(latest);
        }

        return Status::OK;

    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ Export failed: {}", e.what());
        response->set_success(false);
        response->set_message("Internal server error: " + std::string(e.what()));
        return Status::OK;
    } catch (...) {
        SPDLOG_ERROR("❌ Export failed with unknown error.");
        response->set_success(false);
        response->set_message("Unknown server error.");
        return Status::OK;
    }
}

void runReportExportServer(std::shared_ptr<ReportGenerator> generator, const std::string& address) {
    try {
        ReportExportServiceImpl service(generator);

        ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<Server> server(builder.BuildAndStart());
        SPDLOG_INFO("🚀 gRPC server started on {}", address);
        server->Wait();
    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("💥 Failed to start gRPC server: {}", e.what());
    } catch (...) {
        SPDLOG_CRITICAL("💥 Unknown error while starting gRPC server.");
    }
}

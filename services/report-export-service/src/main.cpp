#include "config_loader.hpp"
#include "report_generator.hpp"
#include "report_export_server.hpp"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <memory>

int main() {
    try {
        // Setup logging
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
        SPDLOG_INFO("📄 Report Export Service Starting...");

        // Load configuration
        if (!ConfigLoader::loadEnv(".env")) {
            SPDLOG_CRITICAL("❌ Failed to load .env. Aborting.");
            return EXIT_FAILURE;
        }

        std::string exportPath = ConfigLoader::get("EXPORT_OUTPUT_PATH", "data/exports");
        std::string grpcPort = ConfigLoader::get("GRPC_PORT", "50059");
        std::string grpcAddress = "0.0.0.0:" + grpcPort;

        std::filesystem::create_directories(exportPath);
        auto generator = std::make_shared<ReportGenerator>(exportPath);

        // Start gRPC Server
        runReportExportServer(generator, grpcAddress);
    }
    catch (const std::exception& e) {
        SPDLOG_CRITICAL("💥 Unhandled exception in main: {}", e.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        SPDLOG_CRITICAL("💥 Unknown fatal error occurred in main.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

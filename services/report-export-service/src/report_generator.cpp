#include "report_generator.hpp"
#include "csv_writer.hpp"
#include "pdf_writer.hpp"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <openssl/sha.h>

namespace fs = std::filesystem;

namespace {

// Simple SHA-1 hash of userId + type + time
std::string generateHash(const std::string& base) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(base.c_str()), base.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < 6; ++i)  // Only take 6 chars for filename brevity
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

std::string buildFileName(const std::string& userId, const std::string& type, const std::string& ext) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream timestamp;
    timestamp << std::put_time(std::localtime(&now), "%Y%m%d%H%M%S");

    std::string hash = generateHash(userId + type + timestamp.str());
    return userId + "_" + type + "_" + timestamp.str() + "_" + hash + "." + ext;
}

} // namespace

ReportGenerator::ReportGenerator(const std::string& outputDir)
    : outputDir_(outputDir) {
    fs::create_directories(outputDir_);
}

bool ReportGenerator::generateCSVReport(const std::string& userId,
                                        const nlohmann::json& data,
                                        const std::string& type) {
    if (!validateData(data, type)) return false;
    std::string filename = outputDir_ + "/" + buildFileName(userId, type, "csv");
    return CSVWriter::write(filename, data);
}

bool ReportGenerator::generatePDFReport(const std::string& userId,
                                        const nlohmann::json& data,
                                        const std::string& type) {
    if (!validateData(data, type)) return false;
    std::string filename = outputDir_ + "/" + buildFileName(userId, type, "pdf");
    return PDFWriter::write(filename, data);
}

bool ReportGenerator::validateData(const nlohmann::json& data, const std::string& type) {
    if (!data.is_array()) {
        SPDLOG_ERROR("❌ ReportGenerator: data is not a JSON array.");
        return false;
    }
    if (data.empty()) {
        SPDLOG_ERROR("❌ ReportGenerator: data array is empty.");
        return false;
    }
    if (data.size() > 10000) {
        SPDLOG_ERROR("❌ ReportGenerator: row limit exceeded ({} rows)", data.size());
        return false;
    }

    // Minimal schema check for type
    const auto& first = data[0];
    if (!first.contains("timestamp") || !first.contains("amount") || !first.contains("symbol")) {
        SPDLOG_ERROR("❌ ReportGenerator: missing required keys in row.");
        return false;
    }

    return true;
}

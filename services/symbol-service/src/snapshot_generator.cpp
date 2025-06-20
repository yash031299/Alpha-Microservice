#include "snapshot_generator.hpp"
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

std::string SnapshotGenerator::generateJsonL1(const std::string& symbol,
                                              const std::vector<OrderLevel>& levels) {
    std::ostringstream oss;

    try {
        oss << "{";
        oss << "\"symbol\":\"" << symbol << "\",";

        oss << "\"bid\":";
        if (!levels.empty()) {
            oss << std::fixed << std::setprecision(2)
                << "{\"price\":" << levels[0].price << ",\"qty\":" << levels[0].quantity << "}";
        } else {
            SPDLOG_WARN("generateJsonL1: No bid data for {}", symbol);
            oss << "null";
        }

        oss << ",\"ask\":";
        if (levels.size() > 1) {
            oss << std::fixed << std::setprecision(2)
                << "{\"price\":" << levels[1].price << ",\"qty\":" << levels[1].quantity << "}";
        } else {
            SPDLOG_WARN("generateJsonL1: No ask data for {}", symbol);
            oss << "null";
        }

        oss << "}";

    } catch (const std::exception& e) {
        SPDLOG_ERROR("generateJsonL1 failed for {}: {}", symbol, e.what());
        return "{\"symbol\":\"" + symbol + "\",\"bid\":null,\"ask\":null}";
    } catch (...) {
        SPDLOG_ERROR("Unknown error in generateJsonL1 for {}", symbol);
        return "{\"symbol\":\"" + symbol + "\",\"bid\":null,\"ask\":null}";
    }

    return oss.str();
}

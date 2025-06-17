#include "realized_pnl_store.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

RealizedPnlStore::RealizedPnlStore() {
    filePath_ = "realized_pnl.json"; // configurable if needed
}

RealizedPnlStore& RealizedPnlStore::getInstance() {
    static RealizedPnlStore instance;
    return instance;
}

void RealizedPnlStore::appendPnl(const std::string& user,
                                  const std::string& symbol,
                                  double pnl,
                                  const std::string& timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);

    nlohmann::json j;
    j["user"] = user;
    j["symbol"] = symbol;
    j["pnl"] = pnl;
    j["timestamp"] = timestamp;

    std::ofstream out(filePath_, std::ios::app);
    out << j.dump() << std::endl;
}

#ifndef REALIZED_PNL_STORE_HPP
#define REALIZED_PNL_STORE_HPP

#include <string>
#include <mutex>

class RealizedPnlStore {
public:
    static RealizedPnlStore& getInstance();
    void appendPnl(const std::string& user,
                   const std::string& symbol,
                   double pnl,
                   const std::string& timestamp);

private:
    RealizedPnlStore();
    std::mutex mutex_;
    std::string filePath_;
};

#endif // REALIZED_PNL_STORE_HPP

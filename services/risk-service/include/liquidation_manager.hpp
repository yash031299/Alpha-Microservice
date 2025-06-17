#pragma once
#include <string>

namespace rms {

class LiquidationManager {
public:
    void liquidate(const std::string& userId,
                   const std::string& symbol,
                   double positionSize,
                   int side,
                   double ltp);
};

}  // namespace rms

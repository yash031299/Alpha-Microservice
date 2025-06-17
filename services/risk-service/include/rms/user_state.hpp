#pragma once
#include <string>

namespace rms {

enum class PositionSide {
    Long = 1,
    Short = -1,
    None = 0
};

struct UserState {
    std::string userId;
    std::string symbol = "BTC-USDT";
    double entryPrice = 0.0;
    double positionSize = 0.0;
    PositionSide side = PositionSide::None;
    double leverage = 1.0;
    double walletBalance = 0.0;
};

}  // namespace rms

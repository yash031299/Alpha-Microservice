#pragma once
#include "core/user.hpp"
#include <memory>

class ILiquidationStrategy {
public:
    virtual void apply(user& usr) = 0;
    virtual ~ILiquidationStrategy() = default;
    std::shared_ptr<ILiquidationStrategy> createDefaultStrategy();
};

std::shared_ptr<ILiquidationStrategy> createDefaultStrategy();
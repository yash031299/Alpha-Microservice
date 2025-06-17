#pragma once
#include "core/user.hpp"
#include <optional>

class IRedisClient {
public:
    virtual std::optional<std::shared_ptr<user>> getUser(const std::string& key) = 0;
    virtual ~IRedisClient() = default;
    virtual bool saveUser(const user& usr) = 0;

};

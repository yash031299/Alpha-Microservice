#ifndef FUNDING_PUBLISHER_HPP
#define FUNDING_PUBLISHER_HPP

#include "funding_calculator.hpp"
#include <hiredis/hiredis.h>
#include <mutex>
#include <string>

class FundingPublisher {
public:
    FundingPublisher();
    ~FundingPublisher();

    void publish(const std::string& symbol, const FundingRate& rate);

private:
    redisContext* connect();
    redisContext* conn_;
    std::string redisHost_;
    int redisPort_;
    std::mutex mtx_;
};

#endif // FUNDING_PUBLISHER_HPP

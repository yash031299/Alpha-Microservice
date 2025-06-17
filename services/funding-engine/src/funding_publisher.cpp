#include "funding_publisher.hpp"
#include "config_loader.hpp"

#include <spdlog/spdlog.h>
#include <sstream>

FundingPublisher::FundingPublisher() {
    redisHost_ = ConfigLoader::getEnv("REDIS_HOST", "localhost");
    redisPort_ = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));
    conn_ = connect();
}

FundingPublisher::~FundingPublisher() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (conn_) {
        redisFree(conn_);
        conn_ = nullptr;
    }
}

redisContext* FundingPublisher::connect() {
    redisContext* c = redisConnect(redisHost_.c_str(), redisPort_);
    if (!c || c->err) {
        SPDLOG_ERROR("❌ Redis connection failed: {}", c ? c->errstr : "Unknown error");
        if (c) redisFree(c);
        return nullptr;
    }
    SPDLOG_INFO("✅ Connected to Redis at {}:{}", redisHost_, redisPort_);
    return c;
}

void FundingPublisher::publish(const std::string& symbol, const FundingRate& rate) {
    std::lock_guard<std::mutex> lock(mtx_);

    if (!conn_) {
        conn_ = connect();
        if (!conn_) {
            SPDLOG_ERROR("Redis unavailable. Skipping funding publish for {}", symbol);
            return;
        }
    }

    std::ostringstream cmd;
    cmd << "XADD FUNDING:STREAM:" << symbol << " * "
        << "rate " << rate.rate << " "
        << "longOi " << rate.longOi << " "
        << "shortOi " << rate.shortOi << " "
        << "ts \"" << rate.timestamp << "\"";

    redisReply* reply = (redisReply*)redisCommand(conn_, cmd.str().c_str());

    if (!reply) {
        SPDLOG_ERROR("❌ Failed to publish funding for {}: {}", symbol, conn_->errstr);
        redisFree(conn_);
        conn_ = nullptr;
        return;
    }

    SPDLOG_INFO("📤 Published funding for {}: Rate {:.6f}, Long {:.2f}, Short {:.2f}",
                symbol, rate.rate, rate.longOi, rate.shortOi);

    freeReplyObject(reply);
}

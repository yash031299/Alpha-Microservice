#include "order_parser.hpp"
#include <rapidjson/document.h>
#include <stdexcept>
#include <spdlog/spdlog.h>

Order OrderParser::parse(const std::string& jsonString) {
    rapidjson::Document d;
    d.Parse(jsonString.c_str());

    if (d.HasParseError() || !d.HasMember("orderId") || !d.HasMember("symbol")) {
        throw std::runtime_error("Invalid order JSON");
    }

    Order order;
    order.orderId = d["orderId"].GetString();
    order.userId = d["userId"].GetString();
    order.symbol = d["symbol"].GetString();
    order.price = d["price"].GetDouble();
    order.quantity = d["quantity"].GetDouble();
    order.isBuy = (std::string(d["direction"].GetString()) == "BUY");

    // 🧠 Optional fields with fallback
    if (d.HasMember("trigger_price")) order.triggerPrice = d["trigger_price"].GetDouble();
    if (d.HasMember("reduce_only")) order.reduceOnly = d["reduce_only"].GetInt() == 1;
    if (d.HasMember("post_only")) order.postOnly = d["post_only"].GetBool();

    // Parse ordertype
    if (d.HasMember("ordertype")) {
        std::string type = d["ordertype"].GetString();
        if (type == "LIMIT") order.orderType = OrderType::LIMIT;
        else if (type == "MARKET") order.orderType = OrderType::MARKET;
        else if (type == "STOP_LIMIT") order.orderType = OrderType::STOP_LIMIT;
        else SPDLOG_WARN("⚠️ Unknown ordertype: {}", type);
    }

    // Parse TIF
    if (d.HasMember("tif")) {
        std::string tif = d["tif"].GetString();
        if (tif == "IOC") order.tif = TimeInForce::IOC;
        else if (tif == "FOK") order.tif = TimeInForce::FOK;
        else order.tif = TimeInForce::GTC; // default
    }

    return order;
}

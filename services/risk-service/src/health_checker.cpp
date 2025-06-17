#include "config_loader.hpp"
#include "redis_retry.hpp"
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <spdlog/spdlog.h>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void doSession(tcp::socket socket, std::string redisHost, int redisPort) {
    try {
        beast::flat_buffer buffer;

        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        http::response<http::string_body> res;
        res.version(req.version());
        res.keep_alive(false);

        if (req.target() == "/healthz") {
            redisContext* ctx = safeRedisConnect(redisHost, redisPort);
            if (!ctx || ctx->err) {
                if (ctx) redisFree(ctx);
                res.result(http::status::internal_server_error);
                res.body() = "Redis not OK";
            } else {
                redisFree(ctx);
                res.result(http::status::ok);
                res.body() = "OK";
            }
        } else {
            res.result(http::status::not_found);
            res.body() = "Not Found";
        }

        res.set(http::field::content_type, "text/plain");
        res.prepare_payload();
        http::write(socket, res);

    } catch (const std::exception& e) {
        SPDLOG_ERROR("Health check session error: {}", e.what());
    }
}

int startHealthCheckServer() {
    try {
        ConfigLoader::loadEnv(".env");
        std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        int redisPort = std::stoi(ConfigLoader::getEnv("REDIS_PORT", "6379"));
        int listenPort = std::stoi(ConfigLoader::getEnv("HEALTH_PORT", "8082"));

        std::thread([redisHost, redisPort, listenPort] {
            try {
                net::io_context ioc{1};
                tcp::acceptor acceptor(ioc, {tcp::v4(), static_cast<unsigned short>(listenPort)});
                SPDLOG_INFO("🌡️ Boost HTTP /healthz running on port {}", listenPort);

                for (;;) {
                    tcp::socket socket{ioc};
                    acceptor.accept(socket);
                    std::thread{doSession, std::move(socket), redisHost, redisPort}.detach();
                }
            } catch (const std::exception& ex) {
                SPDLOG_CRITICAL("❌ Health server crash: {}", ex.what());
            }
        }).detach();

        return 0;

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("Failed to start health server: {}", e.what());
        return 1;
    }
}

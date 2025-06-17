#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <thread>
#include <spdlog/spdlog.h>
#include "redis_pool_singleton.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void startHealthCheckServer(int port) {
    std::thread([port]() {
        try {
            SPDLOG_INFO("🚀 Entered startHealthCheckServer({})", port);

            net::io_context ioc;
            tcp::acceptor acceptor{ioc, {tcp::v4(), static_cast<unsigned short>(port)}};

            if (!acceptor.is_open()) {
                SPDLOG_ERROR("❌ Acceptor failed to open on port {}", port);
                return;
            }

            SPDLOG_INFO("🩺 Starting /healthz HTTP server on port {}", port);

            for (;;) {
                tcp::socket socket{ioc};
                acceptor.accept(socket);

                beast::flat_buffer buffer;
                http::request<http::string_body> req;
                http::read(socket, buffer, req);

                http::response<http::string_body> res;
                res.version(11);
                res.set(http::field::server, "boost.beast");
                res.keep_alive(false);

                if (req.method() == http::verb::get && req.target() == "/healthz") {
                    redisContext* ctx = nullptr;
                    try {
                        ctx = RedisPoolSingleton::instance().acquire();
                        void* pong = redisCommand(ctx, "PING");
                        bool healthy = pong != nullptr;
                        if (pong) freeReplyObject(pong);
                        RedisPoolSingleton::instance().release(ctx);

                        if (healthy) {
                            res.result(http::status::ok);
                            res.body() = "OK";
                        } else {
                            res.result(http::status::internal_server_error);
                            res.body() = "Redis unhealthy";
                        }

                    } catch (const std::exception& e) {
                        if (ctx) RedisPoolSingleton::instance().release(ctx);
                        SPDLOG_ERROR("❌ Redis health check error: {}", e.what());
                        res.result(http::status::internal_server_error);
                        res.body() = "Redis check exception";
                    }
                } else {
                    res.result(http::status::not_found);
                    res.body() = "404 Not Found";
                }

                res.prepare_payload();
                http::write(socket, res);
            }

        } catch (const std::exception& e) {
            SPDLOG_ERROR("💥 Health check thread exception: {}", e.what());
        }
    }).detach();
}

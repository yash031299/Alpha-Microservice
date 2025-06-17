#include "config_loader.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;

// Handles a single HTTP session
void doSession(tcp::socket socket, std::string redisHost, int redisPort) {
    beast::flat_buffer buffer;
    http::request<http::string_body> req;

    try {
        http::read(socket, buffer, req);

        if (req.method() == http::verb::get && req.target() == "/healthz") {
            redisContext* ctx = redisConnect(redisHost.c_str(), redisPort);
            bool redisOk = ctx && !ctx->err;

            http::response<http::string_body> res{
                redisOk ? http::status::ok : http::status::internal_server_error,
                req.version()
            };
            res.set(http::field::server, "wallet-sync-service");
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = redisOk ? "OK" : "Redis NOT OK";
            res.prepare_payload();

            http::write(socket, res);

            if (ctx) redisFree(ctx);
        }

        socket.shutdown(tcp::socket::shutdown_send);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("❌ Health session error: {}", e.what());
    }
}

// Starts the HTTP health server on a separate thread
int startHealthCheckServer() {
    try {
        std::string redisHost = ConfigLoader::getEnv("REDIS_HOST", "127.0.0.1");
        int redisPort = ConfigLoader::getEnvInt("REDIS_PORT", 6379);
        int listenPort = ConfigLoader::getEnvInt("GRPC_PORT", 8086);

        asio::io_context* ioc = new asio::io_context(1);  // keep pointer so we can capture safely
        tcp::acceptor* acceptor = new tcp::acceptor(*ioc, {tcp::v4(), static_cast<unsigned short>(listenPort)});

        SPDLOG_INFO("🩺 Health check server listening on port {}", listenPort);

        std::thread([ioc]() {
            try {
                ioc->run();
            } catch (const std::exception& e) {
                SPDLOG_CRITICAL("❌ IO context crashed: {}", e.what());
            }
        }).detach();

        std::thread([redisHost, redisPort, acceptor, ioc]() {
            while (true) {
                try {
                    tcp::socket socket{*ioc};
                    acceptor->accept(socket);
                    std::thread(doSession, std::move(socket), redisHost, redisPort).detach();
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("❌ Health check acceptor error: {}", e.what());
                }
            }
        }).detach();

    } catch (const std::exception& e) {
        SPDLOG_CRITICAL("❌ Failed to start health check server: {}", e.what());
    }

    return 0;
}

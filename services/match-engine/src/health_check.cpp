#include "health_check.hpp"
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include "metrics.hpp"


using boost::asio::ip::tcp;

HealthCheckServer::HealthCheckServer(int port)
    : running_(false), port_(port) {}

HealthCheckServer::~HealthCheckServer() {
    stop();
}

void HealthCheckServer::start() {
    running_ = true;

    serverThread_ = std::thread([this]() {
        try {
            boost::asio::io_context io;
            tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port_));

            while (running_) {
                tcp::socket socket(io);
                acceptor.accept(socket);

                boost::asio::streambuf buffer;
                boost::asio::read_until(socket, buffer, "\r\n");
                std::istream stream(&buffer);
                std::string requestLine;
                std::getline(stream, requestLine);

                std::string responseBody;
                if (requestLine.find("GET /metrics") != std::string::npos) {
                    responseBody = Metrics::render();
                } else if (requestLine.find("GET /healthz") != std::string::npos) {
                    responseBody = "healthy";
                } else {
                    responseBody = "not_found";
                }

                std::ostringstream response;
                response << "HTTP/1.1 200 OK\r\n"
                         << "Content-Type: text/plain\r\n"
                         << "Content-Length: " << responseBody.size() << "\r\n"
                         << "Connection: close\r\n\r\n"
                         << responseBody;

                boost::asio::write(socket, boost::asio::buffer(response.str()));
            }
        } catch (const std::exception& e) {
            SPDLOG_ERROR("Health check server error: {}", e.what());
        }
    });

    SPDLOG_INFO("✅ Health check server running on port {}", port_);
}


void HealthCheckServer::stop() {
    running_ = false;
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    SPDLOG_INFO("🛑 Health check server stopped");
}

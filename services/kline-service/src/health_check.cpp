#include "health_check.hpp"
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <thread>
#include <string>
#include <atomic>

extern std::atomic<int> flushCount;
extern std::atomic<double> lastFlushDuration;

void startHealthCheckServer(int port) {
    std::thread([port]() {
        try {
            boost::asio::io_context io;
            boost::asio::ip::tcp::acceptor acceptor(io,
                boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

            SPDLOG_INFO("📡 Health check + metrics server started on port {}", port);

            while (true) {
                boost::asio::ip::tcp::socket socket(io);
                acceptor.accept(socket);

                boost::asio::streambuf request;
                boost::asio::read_until(socket, request, "\r\n");

                std::istream stream(&request);
                std::string request_line;
                std::getline(stream, request_line);

                std::string response_body, content_type = "text/plain";
                if (request_line.find("GET /healthz") != std::string::npos) {
                    response_body = "healthy";
                } else if (request_line.find("GET /metrics") != std::string::npos) {
                    response_body =
                        "# HELP kline_flush_duration Duration of last candle flush\n"
                        "# TYPE kline_flush_duration gauge\n"
                        "kline_flush_duration " + std::to_string(lastFlushDuration.load()) + "\n"
                        "# HELP kline_ohlcv_count Number of candles flushed in last cycle\n"
                        "# TYPE kline_ohlcv_count gauge\n"
                        "kline_ohlcv_count " + std::to_string(flushCount.load()) + "\n";
                } else {
                    response_body = "404 Not Found";
                }

                std::ostringstream response;
                response << "HTTP/1.1 200 OK\r\n"
                         << "Content-Type: " << content_type << "\r\n"
                         << "Content-Length: " << response_body.size() << "\r\n\r\n"
                         << response_body;

                boost::asio::write(socket, boost::asio::buffer(response.str()));
            }
        } catch (const std::exception& e) {
            SPDLOG_ERROR("❌ Health check server failed: {}", e.what());
        }
    }).detach();
}

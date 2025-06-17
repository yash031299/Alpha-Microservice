#include <boost/asio.hpp>
#include <thread>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

using boost::asio::ip::tcp;

void startHealthCheckServer(unsigned short port = 8081) {
    std::thread([port]() {
        try {
            boost::asio::io_context io;
            tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));
            SPDLOG_INFO("📡 Health check server started on port {}", port);

            while (true) {
                tcp::socket socket(io);
                acceptor.accept(socket);

                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 2\r\n\r\nOK";

                boost::system::error_code ignored_error;
                boost::asio::write(socket, boost::asio::buffer(response), ignored_error);
            }

        } catch (const std::exception& e) {
            SPDLOG_ERROR("❌ Health check server crashed: {}", e.what());
        }
    }).detach();
}

#include "http_health_server.hpp"
#include <httplib.h>
#include <spdlog/spdlog.h>
#include <thread>

void startHealthServer(int port) {
    std::thread([port]() {
        httplib::Server svr;

        svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("OK", "text/plain");
        });

        SPDLOG_INFO("📡 Health server listening on 0.0.0.0:{}", port);
        svr.listen("0.0.0.0", port);
    }).detach();
}

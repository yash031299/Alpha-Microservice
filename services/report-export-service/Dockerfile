# ---------- STAGE 1: Build ----------
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake meson ninja-build \
    libspdlog-dev libnlohmann-json-dev pkg-config wkhtmltopdf

WORKDIR /app
COPY . .

RUN meson setup build --buildtype=release
RUN meson compile -C build

# ---------- STAGE 2: Runtime ----------
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libspdlog-dev libnlohmann-json-dev wkhtmltopdf

COPY --from=builder /app/build/report-export-service /usr/local/bin/report-export-service

WORKDIR /usr/local/bin

CMD ["/usr/local/bin/report-export-service"]

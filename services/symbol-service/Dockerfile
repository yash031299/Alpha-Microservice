# ---------- STAGE 1: Build ----------
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake meson ninja-build \
    libspdlog-dev libhiredis-dev pkg-config

WORKDIR /app
COPY . .

RUN meson setup build --buildtype=release
RUN meson compile -C build

# ---------- STAGE 2: Runtime ----------
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y libspdlog-dev libhiredis-dev

COPY --from=builder /app/build/symbol-service /usr/local/bin/symbol-service
COPY .env /usr/local/bin/.env

WORKDIR /usr/local/bin
CMD ["/usr/local/bin/symbol-service"]

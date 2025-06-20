# ---------- STAGE 1: Build ----------
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake meson ninja-build \
    libspdlog-dev libboost-all-dev \
    libhiredis-dev libssl-dev pkg-config \
    libprotobuf-dev protobuf-compiler \
    libgrpc++-dev libgrpc-dev \
    nlohmann-json3-dev redis-server

WORKDIR /app
COPY . .

RUN meson setup build --buildtype=release
RUN meson compile -C build

# ---------- STAGE 2: Runtime ----------
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libspdlog-dev libboost-all-dev \
    libhiredis-dev libprotobuf-dev libgrpc++-dev \
    nlohmann-json3-dev redis-tools

COPY --from=builder /app/build/match-engine /usr/local/bin/match-engine

WORKDIR /usr/local/bin

CMD ["/usr/local/bin/match-engine"]

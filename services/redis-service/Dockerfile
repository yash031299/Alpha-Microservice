# ───── Base: Slim Debian + C++ Toolchain ─────
FROM debian:bullseye-slim AS builder

# ───── Install build tools and dependencies ─────
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    ninja-build \
    pkg-config \
    libboost-system-dev \
    libboost-thread-dev \
    libhiredis-dev \
    libspdlog-dev \
    libnlohmann-json-dev \
    libfmt-dev \
    libssl-dev \
    libprotobuf-dev protobuf-compiler \
    meson \
    && rm -rf /var/lib/apt/lists/*

# ───── Copy source ─────
WORKDIR /app
COPY . .

# ───── Build ─────
RUN meson setup build --buildtype=release && \
    meson compile -C build

# ───── Runtime image ─────
FROM debian:bullseye-slim

# ───── Runtime dependencies only ─────
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libhiredis-dev \
    libspdlog-dev \
    libnlohmann-json-dev \
    libfmt8 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/redis-service /app/redis-service
COPY .env /app/.env

# ───── Entrypoint ─────
ENTRYPOINT ["./redis-service"]

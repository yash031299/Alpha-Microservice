# ─────────────────────────────
# Stage 1: Build
# ─────────────────────────────
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    meson \
    ninja-build \
    pkg-config \
    git \
    libspdlog-dev \
    libboost-all-dev \
    libhiredis-dev

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Configure and build
RUN meson setup build --buildtype=release
RUN meson compile -C build

# ─────────────────────────────
# Stage 2: Runtime
# ─────────────────────────────
FROM ubuntu:22.04

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
    libspdlog-dev \
    libboost-system-dev \
    libhiredis-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Copy built binary
COPY --from=builder /app/build/kline-service /usr/local/bin/kline-service

# Set working directory
WORKDIR /usr/local/bin

# Healthcheck endpoint
EXPOSE 8081

# Default command
CMD ["./kline-service"]

# ───────────────────────────────────────────────
# Stage 1: Builder
# ───────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake meson ninja-build \
    libspdlog-dev libboost-all-dev pkg-config \
    libhiredis-dev

WORKDIR /app
COPY . .

RUN meson setup build --buildtype=release
RUN meson compile -C build

# ───────────────────────────────────────────────
# Stage 2: Runtime
# ───────────────────────────────────────────────
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libspdlog-dev libboost-system-dev libhiredis-dev \
 && apt-get clean

COPY --from=builder /app/build/funding-engine /usr/local/bin/funding-engine
COPY .env /usr/local/bin/.env

WORKDIR /usr/local/bin

# Optional: Docker-native liveness probe
HEALTHCHECK --interval=10s --timeout=3s --start-period=5s CMD curl -f http://localhost:8081/healthz || exit 1

CMD ["./funding-engine"]

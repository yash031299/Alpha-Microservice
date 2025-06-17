# ───────────────────────────────────────────────
# Stage 1: Build
# ───────────────────────────────────────────────
FROM debian:bullseye as builder

RUN apt-get update && apt-get install -y \
    git cmake ninja-build python3-pip build-essential \
    libhiredis-dev libboost-all-dev libspdlog-dev \
    libprometheus-cpp-dev libhttplib-dev

WORKDIR /app
COPY . .

RUN pip3 install conan meson
RUN ./build.sh

# ───────────────────────────────────────────────
# Stage 2: Run
# ───────────────────────────────────────────────
FROM debian:bullseye

RUN apt-get update && apt-get install -y \
    libhiredis0.14 libboost-system-dev libspdlog-dev \
    libprometheus-cpp-dev libhttplib-dev

WORKDIR /app
COPY --from=builder /app/build/liquidation-service .

COPY .env .env

CMD ["./liquidation-service"]

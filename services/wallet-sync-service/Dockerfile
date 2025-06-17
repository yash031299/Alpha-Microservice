# -------- Base Image --------
FROM debian:bullseye

# -------- Install Dependencies --------
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    meson \
    ninja-build \
    libhiredis-dev \
    libspdlog-dev \
    libnlohmann-json-dev \
    libboost-all-dev \
    wget \
    curl \
    ca-certificates \
    git \
    pkg-config

# -------- Setup Workdir --------
WORKDIR /app

# -------- Copy Source --------
COPY . .

# -------- Build Project --------
RUN meson setup build && meson compile -C build

# -------- Run the Binary --------
ENTRYPOINT ["./build/wallet-sync-service"]

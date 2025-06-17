FROM gcc:13

WORKDIR /app

RUN apt-get update && apt-get install -y \
    python3 python3-pip \
    pkg-config libhiredis-dev libprotobuf-dev protobuf-compiler \
    libgrpc++-dev grpc-proto libspdlog-dev libboost-all-dev \
    && pip3 install conan meson ninja \
    && conan profile detect

COPY . .

RUN conan install . --output-folder=build --build=missing

RUN meson setup build --buildtype=release && meson compile -C build

CMD ["./build/order_service"]

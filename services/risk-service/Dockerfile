FROM alpine:latest

RUN apk add --no-cache \
    g++ \
    libc-dev \
    make \
    cmake \
    git \
    boost-dev \
    openssl-dev \
    hiredis-dev \
    spdlog-dev \
    fmt-dev \
    nlohmann-json \
    libexecinfo-dev

WORKDIR /app

COPY . .

RUN mkdir -p build && cd build && \
    cmake .. && \
    make

CMD ["./build/risk-service"]

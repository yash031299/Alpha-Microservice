# 📡 Stream Service

## Overview

The `stream-service` provides **real-time market data broadcasting** by subscribing to Redis Streams for ticks and trades. It enables downstream systems like dashboards, APIs, or external subscribers to receive fresh data with minimal latency.

---

## Responsibilities

- Subscribe to Redis streams (e.g., `FUTURE:TICK:{symbol}`)
- Parse incoming price/volume ticks
- Maintain an internal snapshot of latest tick data
- Optional: Broadcast to WebSocket or gRPC stream clients

---

## Redis Integration

- Uses `XREAD` to listen on:
  ```
  FUTURE:TICK:{symbol}
  ```
- Each message typically includes:
  ```json
  {
    "price": 31200.0,
    "qty": 0.05,
    "timestamp": 1718629200
  }
  ```

---

## Flow

1. Subscribes to stream key(s) using Redis context
2. Receives new messages via blocking `XREAD`
3. Parses JSON fields (price, qty, timestamp)
4. Updates internal cache (L1 store or tick store)
5. Exposes latest tick on-demand or via publish

---

## Used By

- Frontend dashboards (WebSocket, REST gateway)
- Market makers for price sync
- Monitoring and analytics pipelines

---

## Features

- 🔄 Low-latency Redis subscription
- 🔍 Symbol-wise data tracking
- ⚙️ Modular structure to add streaming backends
- 📤 Can emit events to Kafka, WebSocket, or gRPC

---

## Design Goals

- Non-blocking Redis listener (using Boost Asio)
- Per-symbol stream isolation
- Tick ID tracking to prevent duplication
- Fast lookups via in-memory maps

---

## TODOs

- [ ] Add WebSocket broadcast layer
- [ ] Add Prometheus metrics (stream lag, dropped events)
- [ ] Graceful shutdown on signal
- [ ] Handle multiple Redis shards

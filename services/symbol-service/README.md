# 🧭 Symbol Service

## Overview

The `symbol-service` provides **real-time L1 snapshots** (best bid/ask) of a symbol's order book. It is queried by other services like `execution-service`, `order-service`, or any component that needs the most up-to-date market data.

---

## Responsibilities

- Serve Level 1 snapshot for a given symbol
- Return:
  - Best bid price + quantity
  - Best ask price + quantity
  - Timestamp
- Fetch from internal atomic order book or in-memory data stream
- Validate snapshot integrity before responding

---

## gRPC Interface

### RPC

```proto
rpc GetL1 (SymbolRequest) returns (L1Snapshot);
```

### Response Format

```json
{
  "symbol": "BTC-USDT",
  "bid": {
    "price": 24950.0,
    "qty": 0.8
  },
  "ask": {
    "price": 25000.0,
    "qty": 0.6
  },
  "timestamp": "2025-06-18T11:00:00Z"
}
```

---

## Internal Structure

- Order book or symbol snapshot is updated by:
  - match-engine
  - Redis subscriber
- Atomic snapshot ensures read safety
- Timestamp helps in latency checks

---

## Consumers

- `execution-service` (uses snapshot for market/SL orders)
- `order-service` (optional validation)
- `audit-log-service` (for post-trade snapshots)
- `kline-service` or `stream-service` (for L1 stream or charting)

---

## Features

- 🔒 Thread-safe L1 snapshot delivery
- 📉 Accurate bid/ask with quantity
- 🧠 Stateless gRPC microservice
- ⏱️ Real-time market responsiveness

---

## TODOs

- [ ] Add L2 snapshot support
- [ ] Integrate timestamp-based caching
- [ ] Emit snapshot lag/latency metrics
- [ ] Enable snapshot streaming via gRPC stream

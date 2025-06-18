# 🛠️ Redis Service

## Overview
The `redis-service` provides a centralized utility for **publishing and subscribing** to Redis topics or streams across Alpha OMS. It is a lightweight helper service that encapsulates Redis pub/sub, stream publishing, and connection pooling.

---

## Responsibilities

- Publish structured JSON messages to Redis channels or streams
- Reusable Redis connection pool for other services
- Provide utility APIs for:
  - `publishTrade()`
  - `publishOrderUpdate()`
  - `streamTickData()`
- Wrap `hiredis` or other Redis clients with retry-safe logic

---

## Usage

### Trade Publisher Example
```cpp
RedisPublisher publisher("tcp://localhost:6379");
publisher.publishTrade({
    "orderId": "abc123",
    "price": 25000,
    "qty": 0.5
});
```

### Output (Redis Pub/Sub)
```
Channel: trades_channel
Payload: {"orderId":"abc123", "price":25000, "qty":0.5}
```

---

## Internals

- Built using hiredis / cpp-redis wrapper
- RedisConnectionPool class:
  - Creates N reusable connections
  - Thread-safe via mutex/lock
- Configurable Redis host/port via `.env`

---

## Features

- 🔁 Publish retries on failure
- 🚦 Configurable stream vs pubsub mode
- 🧵 Thread-safe pool with N connections
- 📦 JSON serialization with schema checks

---

## Consumers

- `execution-service` → publishes `STREAM:TRADE`
- `match-engine` → publishes order matches
- `wallet-sync-service`, `stream-service` → listen to Redis stream

---

## TODOs

- [ ] Add subscriber wrapper for Redis stream consumption
- [ ] Integrate metrics: publish lag, error count
- [ ] Add graceful pool shutdown handler

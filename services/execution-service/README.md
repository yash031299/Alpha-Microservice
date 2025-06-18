# 🚀 Execution Service

## Overview

The `execution-service` is a production-grade microservice that:

- Listens for **market** or **stop-loss (SL)** orders via gRPC
- Fetches latest order book snapshots (from `symbol-service`)
- Matches orders and calculates fills
- Publishes **executed trades** to Redis (`STREAM:TRADE`)
- Sends **order updates** to Redis and other downstream services

---

## Responsibilities

- Handle market and SL order execution outside the match engine
- Maintain trade consistency and average price logic
- Publish successful trades to Redis stream
- Update order status in Redis (via RedisUpdate pool)

---

## Execution Flow

1. **Receive Order**

   - Accepts gRPC request containing order payload (market or SL)

2. **Fetch Order Book**

   - Requests L1 snapshot from `symbol-service` gRPC endpoint
   - Snapshot includes:
     - Top of book: best bid/ask and their quantities

3. **Execute Order**

   - BUY market → match against ask side
   - SELL market → match against bid side
   - Calculates:
     - Weighted average execution price
     - Total filled quantity

4. **Trade Publication**

   - Trade JSON published to:
     ```
     STREAM:TRADE
     ```
   - Format: `{ user, symbol, price, qty, side }`

5. **Status Update**
   - Redis `HMSET` update:
     - Execution state: FILLED or PARTIAL
     - Remaining quantity
     - Average executed price

---

## Redis Integration

- Publishes trade events for:
  - wallet-sync-service
  - kline-service
  - pnl-service
- Redis client pool via `RedisConnectionPool`
- Uses async `boost::asio` thread pool to enqueue work

---

## gRPC Endpoint

- Exposes:
  - `ExecuteMarketOrder(OrderRequest)`
  - `ExecuteSLOrder(OrderRequest)`
- Accepts:
  - symbol
  - user_id
  - quantity
  - direction
  - reduce_only flag
  - optional trigger price (for SL orders)

---

## Production Features

- 🔄 Retry-safe trade publishing
- ⛑️ Graceful error handling and logging
- 🧮 Clean separation between price discovery and execution logic
- 🧪 Ready for unit and integration testing

---

## TODOs

- [ ] Expose gRPC healthcheck endpoint
- [ ] Integrate Prometheus metrics
- [ ] Support async cancel/modify in future

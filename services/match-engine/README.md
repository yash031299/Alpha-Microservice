# ⚔️ Match Engine Service

## Overview

The `match-engine` is the heart of the Alpha OMS. It performs **high-performance matching** of limit orders using an in-memory order book. It maintains real-time bid/ask levels, handles cancels/modifies, and emits trades once matched.

---

## Responsibilities

- Maintain a live in-memory order book per symbol
- Match limit orders (BUY/SELL) using price-time priority
- Handle:
  - Order insertion (maker)
  - Taker orders (crosses the book)
  - Order modification and cancellation
- Emit trades to downstream services (execution, audit, stream)

---

## Flow

### 1. **Limit Order Processing**

- When a limit order is received:
  - If it crosses the opposite book → immediate execution (Taker)
  - If not → stored in the order book (Maker)

### 2. **Matching Logic**

- Sorted maps used:
  - `buy_orders`: sorted in descending price order
  - `sell_orders`: sorted in ascending price order
- Execution loop:
  - Match orders based on best price
  - Track:
    - `exe_quantity`
    - `avg_exe_price`
  - Emit `FILLED` or `PARTIAL` events

### 3. **Order Management**

- Stores orders in:
  - `order_lookup` for quick cancellation or modification
- Can:
  - Modify price/qty of live orders
  - Cancel by `orderId`

### 4. **Concurrency Safety**

- Atomic `shared_ptr<OrderBook>` used to safely share state
- Separate mutexes:
  - `buy_orders_mtx`, `sell_orders_mtx`, `order_lookup_mtx`

---

## Key Interfaces

- `SubmitOrder(OrderRequest)` — handle new limit order
- `CancelOrder(CancelRequest)` — cancel live order
- `ModifyOrder(ModifyRequest)` — modify live order
- `GetOrderBookSnapshot()` — return L2 order book

---

## Dependencies

- Redis (optional for updates)
- Internal: `executor.h`, `redis_stream.h`, `symbol_manager.h`
- `spdlog`, `boost`, `rapidjson`

---

## Trade Publication

When a match happens:

- Create a trade event:
  ```json
  {
    "makerOrderId": "...",
    "takerOrderId": "...",
    "symbol": "BTC-USDT",
    "price": 30000.0,
    "quantity": 0.5,
    "timestamp": "2025-06-18T10:00:00Z"
  }
  ```
- Published to:
  ```
  STREAM:TRADE
  ```
- Picked by:
  - wallet-sync-service
  - pnl-service
  - stream-service

---

## Design Goals

- ⏱️ Low-latency matching (O(logN) insert/delete)
- 🧵 Thread-safe atomic state sharing
- 🪝 Event-driven matching with clean separation of concerns

---

## Future Enhancements

- [ ] L3 order book (per order level detail)
- [ ] Latency tracking via Prometheus
- [ ] Redis-based backup of in-memory book
- [ ] Multi-symbol matching engine (current = 1 per symbol)

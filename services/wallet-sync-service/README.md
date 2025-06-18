# 🔁 Wallet Sync Service

## Overview
The `wallet-sync-service` listens to **trade events from Redis Streams** and applies **realized PnL** updates to user wallets in near real-time. It ensures consistency between trade outcomes and wallet balances, even during bursts of trading activity.

---

## Responsibilities

- Listen to Redis Stream (`STREAM:TRADE`)
- Parse trade JSON (user, qty, price, direction)
- Compute realized PnL if applicable
- Call `pnl-service` gRPC to update wallet
- Log results for each wallet update

---

## Redis Integration

- Listens on:
  ```
  STREAM:TRADE
  ```
  Format:
  ```json
  {
    "user": "test-user-1",
    "symbol": "BTC-USDT",
    "price": 30000.0,
    "qty": 0.5,
    "side": "BUY"
  }
  ```

- Acts as an async Redis consumer
- Uses `XREAD` in a blocking loop

---

## Flow

1. Trade executed by match/exec service → pushed to `STREAM:TRADE`
2. `wallet-sync-service` reads event
3. Parses symbol, userId, qty, price, side
4. Calculates realized PnL if previous position exists
5. Calls `pnl-service → UpdateWallet(user_id, pnl)`
6. Logs status

---

## Features

- ⛓️ Real-time trade-to-wallet sync
- 🧠 Stateless service (pulls data as needed)
- 🔄 Resilient to duplicate events
- 🧪 Built for high throughput

---

## Failure Handling

- If Redis event is malformed → log & skip
- If gRPC to pnl-service fails → retry/queue (future)
- Graceful shutdown via SIGINT handling

---

## Consumers

- Indirect (Redis updated by `pnl-service`)
- User Service loads updated balances periodically

---

## TODOs

- [ ] Add Prometheus metrics for sync delay
- [ ] Add PnL threshold alerting
- [ ] Add fallback buffer for retry on gRPC failure

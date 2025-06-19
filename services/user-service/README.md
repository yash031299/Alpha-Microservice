# 🧍‍♂️ User Service

## Overview

The `user-service` manages **user wallet balances**, **positions**, and handles periodic margin and liquidation checks. It operates as a standalone microservice and interacts with Redis for persistent storage and update.

---

## Responsibilities

- Load and maintain user wallet balances from Redis
- Load and maintain user metadata (KYC, leverage, risk tier)
- Provide gRPC APIs for wallet balance and metadata access
- Apply wallet delta updates (margin funding, PnL)
- Emit margin check events to Redis when wallet changes
- Periodically flush wallet state back to Redis

---

## Architecture & Flow

1. **Bootstrapping**

   - Loads wallet balances and user metadata from Redis.
   - Starts Redis subscriber for `MARGIN_UPDATES` channel.
   - Starts flush scheduler to persist updated balances.

2. **Wallet Update API**

   - gRPC endpoint `UpdateWallet()` applies balance delta.
   - Emits `QUEUE:MARGIN:CHECK:{user_id}` to Redis for downstream risk-service to verify equity and initiate liquidation if needed.

3. **Margin Listener**

   - Subscribes to `MARGIN_UPDATES` Redis Pub/Sub channel.
   - Applies margin deltas (e.g., realized PnL, funding) to in-memory wallet.

4. **Scheduler**

   - Every 5 seconds, all in-memory balances are flushed back to Redis using `HSET`.

5. **Healthcheck**

   - Logs service liveness every 15 seconds using `spdlog`.

---

## Redis Keys Used

- `user_wallets`: Redis hash storing wallet balances, keyed by `user_id`.
- `user_metadata`: Redis hash with values like:
  ```json
  "test-user-1": "kyc:true,risk:high,lev:50"
  ```
- `Emits margin check on`: LPUSH QUEUE:MARGIN:CHECK:`user_id` `<user_id>`

## Config/Dependencies

- Redis
- `spdlog` for structured logging
- Thread-safe design with mutexes and atomic flags
- Periodic flushing using `std::thread`

---

## Production-Ready Features

- 🔁 Thread-safe liquidation with atomic flags
- 🧮 Accurate MMR, leverage, and equity calculations
- 🧠 Background scheduler
- 🧯 Graceful stop using atomic `running` flag

---

## TODOs (optional)

- Replace Redis direct string parsing with structured JSON
- gRPC wrapper for balance APIs (Get/Set)
- Alert and audit hooks for liquidation events

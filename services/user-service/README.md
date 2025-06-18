# 🧍‍♂️ User Service

## Overview

The `user-service` manages **user wallet balances**, **positions**, and handles periodic margin and liquidation checks. It operates as a standalone microservice and interacts with Redis for persistent storage and update.

---

## Responsibilities

- Load and maintain user wallet and position data from Redis
- Perform periodic risk calculations (margin, unrealized PnL)
- Calculate liquidation prices for margin positions
- Initiate liquidation when user's margin falls below thresholds
- Periodically flush updates to Redis

---

## Architecture & Flow

1. **Bootstrapping**

   - Loads wallet balances and user metadata from Redis.
   - Initializes thread pool for risk calculation.

2. **Scheduler**

   - Every 500ms, `calculation()` is triggered in the background.
   - This updates margin status and evaluates liquidation conditions.

3. **Liquidation Trigger**

   - If a user's equity falls below maintenance margin × (1 + buffer),
     liquidation starts in `userManager::liquidation(user&)`.

4. **Position Sorting**

   - Positions are sorted by worst PnL (lowest first).
   - Liquidation occurs until user margin is healthy again.

5. **Liquidation Price Calculation**
   - Uses:
     ```
     liq_price = avg_price ± loss / quantity
     ```
     Based on leverage, notional, and MMR (maintenance margin rate).

---

## Redis Keys Used

- `USER:{user_id}`: Contains wallet and position strings
- `pos:{symbol}` fields store position tuples like:
  quantity, avg_price, leverage, notional, pnl, liquidation_price

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

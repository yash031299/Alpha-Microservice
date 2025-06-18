# 💸 Funding Engine Service

## Overview
The `funding-engine` calculates and distributes **funding payments** between long and short position holders based on open interest imbalance. It is used in perpetual futures trading to keep contract prices aligned with the spot market.

---

## Responsibilities

- Periodically calculate funding rates for each symbol
- Determine net open interest (long vs short)
- Compute funding amount to be exchanged
- Push funding updates to Redis
- Optionally call pnl-service to credit/debit users

---

## Funding Logic

1. For each symbol:
   - Fetch long and short open interest
   - Calculate imbalance ratio
   - Funding rate = function(imbalance, volatility, clamp limits)
   - Apply funding every interval (e.g., 8h or 1h)

2. Net PnL = ± (notional × funding rate)

---

## Example Calculation

```text
Funding Rate = 0.01% (positive → longs pay shorts)

User A: +10,000 USDT position (long) → pays 1 USDT
User B: -10,000 USDT position (short) → receives 1 USDT
```

---

## Redis Usage

- Publishes:
  ```
  FUNDING:{symbol}:{interval}
  ```
  Includes:
  - Symbol
  - Rate
  - Timestamp
  - Paid/received per user

- Optionally triggers:
  - pnl-service gRPC to update wallets

---

## Trigger

- Timer-based scheduler (every 8h or configured)
- Can be triggered manually or via CLI in dev

---

## Design Features

- 📊 Per-symbol, time-based funding engine
- 🔁 Periodic flush of funding activity
- 🔒 Thread-safe scheduler with retry on fail
- 📉 Built for dynamic funding rate curves

---

## TODOs

- [ ] Add historical funding rate tracking
- [ ] Store to database (funding payments ledger)
- [ ] Integrate with alert-service for negative funding spikes
- [ ] Add WebSocket feed for UI display


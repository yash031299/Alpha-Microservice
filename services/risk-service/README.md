# 📉 Risk Service (RMS)

## Overview
The `risk-service` performs **real-time risk evaluation** for user positions. It calculates margin usage, liquidation thresholds, and equity in response to trades, funding, or manual triggers. It ensures users are compliant with risk parameters like maintenance margin and leverage.

---

## Responsibilities

- Evaluate user equity vs. required margin
- Trigger liquidation if equity < maintenance margin
- Calculate:
  - Unrealized PnL
  - Initial margin requirement
  - Maintenance margin requirement
- Read latest LTP from Redis Stream (via `XREAD`)
- Serve risk checks via gRPC

---

## Flow

1. Triggered by:
   - Redis queue: `QUEUE:MARGIN:CHECK:{symbol}`
   - Manual or periodic scheduler

2. Loads user data:
   - Positions: size, avg price, leverage
   - Wallet: available balance

3. Reads latest LTP for each symbol:
   - `FUTURE:TICK:{symbol}` Redis stream

4. Computes:
   - Unrealized PnL = (LTP - entry price) × qty
   - Equity = Wallet + ∑ (Unrealized PnL)
   - Margin = Notional / leverage
   - Triggers liquidation if equity < Maintenance Margin

---

## Redis Keys Used

- `USER:{user_id}` → loads wallet & positions
- `FUTURE:TICK:{symbol}` → LTP source
- `QUEUE:MARGIN:CHECK:*` → triggers for evaluation

---

## gRPC API (Future)

- Check user margin safety
- Return risk health score
- Schedule async checks per symbol/user

---

## Design Features

- 🧠 Accurate financial computation using correct formulas
- 🧵 Thread pool based concurrency (Boost Asio or custom)
- 🔒 Crash-safe with try/catch blocks
- ⛑️ Designed to work under live load

---

## Risk Formulas Used

- **Equity** = Wallet + ∑ PnL
- **MMR** (Maintenance Margin Requirement) = Notional × MMR%
- **Trigger Condition**:
  ```cpp
  if (Equity < Maintenance Margin)
      → Liquidation triggered
  ```

---

## TODOs

- [ ] Add Prometheus metrics for equity/risk distribution
- [ ] Add audit logs for every margin decision
- [ ] Move margin thresholds to symbol-registry-service config

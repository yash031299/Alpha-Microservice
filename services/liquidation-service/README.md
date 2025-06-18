# ☠️ Liquidation Service

## Overview
The `liquidation-service` handles **forced position closures** for users whose accounts fall below maintenance margin. It is a critical safeguard against negative equity and systemic risk in the exchange.

---

## Responsibilities

- Consume liquidation requests from Redis queue
- Fetch user wallet & position state from Redis
- Sort positions by worst PnL (most at-risk)
- Liquidate positions until margin health is restored
- Update Redis with new user state
- Notify downstream (e.g., audit-log, funding-service)

---

## Trigger Source

- Redis queue:
  ```
  QUEUE:LIQUIDATE
  ```
  Triggered by:
  - `risk-service`
  - Manual admin flag
  - Monitoring services

---

## Flow

1. UserId is read from `QUEUE:LIQUIDATE`
2. Load:
   - `USER:{user_id}` from Redis
   - All positions and wallet
3. Calculate:
   - MMR, Notional, Liquidation price
4. Sort positions (worst to best PnL)
5. For each position:
   - Reduce quantity
   - Calculate realized PnL
   - Apply new liquidation price
   - Update Redis immediately

---

## Redis Keys

- `USER:{user_id}` — modified directly after liq
- Fields updated:
  - `wallet`
  - `pos:{symbol}` → updated or cleared

---

## Position Format (example):
```
quantity, avg_price, leverage, notional, pnl, liq_price
```

---

## Design Features

- 🔄 Atomic flag to prevent double-liquidation
- 🧠 Intelligent liquidation order (worst PnL first)
- 🧪 Tested against undercollateralized edge cases
- 🔁 Retry-safe with logging for fallback

---

## Logging

- Logs:
  - Start and end of liquidation
  - Position(s) closed
  - Wallet before and after
  - New margin health status

---

## TODOs

- [ ] Emit audit logs via gRPC
- [ ] Integrate with order-service to cancel open orders
- [ ] Use symbol-registry-service for dynamic MMR%

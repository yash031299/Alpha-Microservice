# 📊 PnL Service

## Overview
The `pnl-service` handles **realized profit and loss (PnL)** updates after a trade is executed. It is responsible for adjusting a user's wallet balance based on trade outcomes. This service ensures post-trade financial updates are accurate and compliant with risk calculations.

---

## Responsibilities

- Accept realized PnL via gRPC (`UpdateWallet`)
- Adjust wallet balance in Redis
- Emit structured logs and events for audit/report
- Serve as downstream processor from `execution-service` or `wallet-sync`

---

## gRPC Interface

### RPC
```proto
rpc UpdateWallet (WalletRequest) returns (WalletResponse);
```

### Example Request
```json
{
  "user_id": "test-user-1",
  "amount": 125.50
}
```

### Response
```json
{
  "success": true,
  "message": "Wallet updated"
}
```

---

## Redis Keys

- Updates:
  ```
  USER:{user_id}
  ```
- Increments/decrements wallet key based on trade PnL
- Also supports funding fee adjustments or rebates

---

## Flow

1. Trade is executed (filled) by execution-service
2. Trade info includes realized PnL for taker/maker
3. pnl-service receives call → updates Redis
4. Emits success/failure response for retry logic

---

## Used By

- `execution-service`
- `wallet-sync-service` (sometimes as bulk mode)
- `report-export-service` (indirectly via history)

---

## Features

- 🔒 Thread-safe wallet update logic
- 📉 Realized PnL and funding updates
- 🔁 Retry-tolerant Redis ops with confirmation
- 🧾 Auditable via optional log hook

---

## TODOs

- [ ] Add metrics for net user profitability
- [ ] Extend to unrealized PnL on request
- [ ] Optional Redis TTL for dev/staging

# 🧮 Accounting Service

## Overview
The `accounting-service` manages **financial tracking and ledger consistency** for all cash flows in the trading system. It ensures that every wallet update, trade, funding, fee, or liquidation event is properly logged into an internal ledger for audit and compliance.

---

## Responsibilities

- Maintain internal double-entry accounting for:
  - Trades
  - PnL
  - Funding
  - Fees
  - Withdrawals/Deposits
- Track all wallet changes as accounting journal entries
- Validate that system remains financially balanced

---

## Journal Format

Each event is recorded as:
```json
{
  "timestamp": "2025-06-18T12:34:00Z",
  "user_id": "user123",
  "type": "trade_pnl",
  "symbol": "BTC-USDT",
  "amount": -50.25,
  "wallet_before": 1020.75,
  "wallet_after": 970.50
}
```

---

## Flow

1. Subscribed to:
   - Redis Stream (`STREAM:TRADE`)
   - Redis Queue (`QUEUE:ACCOUNTING`)
   - Optional gRPC input

2. For each financial event:
   - Validate wallet change
   - Persist entry in internal in-memory journal
   - Optional: write to file or DB

3. Optional:
   - Daily reconciliation check
   - Export to external accounting tools

---

## Features

- 📒 Double-entry ledger-style journal
- 🧾 Tracks wallet deltas and source events
- ✅ Periodic reconciliation (net cash balance check)
- 🧠 Data export compatible with accounting tools (Tally, Excel)

---

## Consumers

- Admin audit panel
- Compliance exports
- `report-export-service` (aggregated data)
- Internal CLI for forensic analysis

---

## TODOs

- [ ] Persist journal to disk or PostgreSQL
- [ ] Integrate with `pnl-service` and `wallet-sync-service`
- [ ] Export full ledger report (CSV, JSON)
- [ ] Add gRPC stream support for high-throughput ingestion

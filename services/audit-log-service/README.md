# 🧾 Audit Log Service

## Overview
The `audit-log-service` records all critical trading and user events into persistent storage for **compliance, debugging, and accountability**. It collects structured logs (trades, orders, funding, liquidations) via gRPC and stores them asynchronously.

---

## Responsibilities

- Receive event logs via gRPC (oneof Trade, Order, Funding, Liquidation)
- Write logs to:
  - File (NDJSON / plain)
  - Redis / Kafka (optional)
  - Future DB integration
- Tag logs with timestamps and source metadata

---

## gRPC Interface

### RPC
```proto
rpc LogEvent (AuditEvent) returns (LogResponse);
```

### Events Supported
```proto
message AuditEvent {
  oneof payload {
    TradeEvent trade = 1;
    OrderEvent order = 2;
    FundingEvent funding = 3;
    LiquidationEvent liquidation = 4;
  }
}
```

---

## Example Log (Trade)
```json
{
  "event": "trade",
  "symbol": "BTC-USDT",
  "price": 30500.5,
  "qty": 0.25,
  "timestamp": "2025-06-18T12:00:00Z"
}
```

---

## Logging Backend

- Default: local file (`logs/audit.ndjson`)
- Optional: Stream to Kafka/Redis for external processing
- Log rotation and compression (future)

---

## Consumers

- Internal compliance dashboards
- Admin inspection tools
- Third-party analytics (if exposed)

---

## Features

- 🔐 Immutable audit log format
- 🧵 Async writer using thread pool
- 📅 Timestamp + event source metadata
- 💾 Fault-tolerant writes with file lock protection

---

## TODOs

- [ ] Add file rotation by date or size
- [ ] Stream logs to external Kafka/Elastic
- [ ] Redact sensitive fields before writing
- [ ] Add indexing for search


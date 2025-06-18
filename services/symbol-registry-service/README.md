# 🧾 Symbol Registry Service

## Overview

The `symbol-registry-service` acts as a **static metadata provider** for all trading symbols in the system. It exposes symbol configuration data such as tick size, leverage limits, precision, and margin requirements via gRPC. This helps other services validate and enforce constraints consistently.

---

## Responsibilities

- Load static config for all symbols at startup (from JSON or YAML)
- Serve metadata via gRPC for:
  - Tick size
  - Leverage
  - Margin (Initial & Maintenance)
  - Precision
  - Base/quote currency

---

## Symbol Metadata Format

Each symbol (e.g., `BTC-USD`) contains:

```json
{
  "symbol": "BTC-USD",
  "tick_size": 0.1,
  "precision": 2,
  "max_leverage": 125,
  "initial_margin": 0.05,
  "maintenance_margin": 0.02,
  "base_currency": "BTC",
  "quote_currency": "USD"
}
```

---

## gRPC API

### RPC

```proto
rpc GetSymbol (SymbolRequest) returns (SymbolMeta);
```

### Consumer Services

- `order-router-service` → Validates user input before forwarding
- `order-service` → Checks tick size, margin
- `rms-service` → Evaluates leverage/margin risk
- `execution-service` → Calculates reduce-only quantity
- `liquidation-service` → Needs margin ratios for liq trigger

---

## Design Features

- ⚙️ In-memory fast lookup (loaded once at startup)
- 🧠 Stateless query handler
- 🔒 Prevents runtime misconfig via strict validation
- 🚀 Efficient symbol onboarding for new markets

---

## Advantages

- 🧱 Single source of truth for symbol constraints
- 🧪 Guarantees consistency across services
- 🛡️ Prevents bad trades or invalid input at entry point

---

## TODOs

- [ ] Add dynamic config reloading (watch .json)
- [ ] Add REST API wrapper (optional)
- [ ] Include funding interval, fees in metadata

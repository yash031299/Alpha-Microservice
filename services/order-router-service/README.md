# 🚦 Order Router Service

## Overview

The `order-router-service` is responsible for **receiving raw user orders** from external clients (e.g., frontend, APIs), performing **basic validations**, and forwarding them to the internal `order-service`. It acts as a secure and scalable **boundary layer** for the OMS.

---

## Responsibilities

- Expose an external gRPC endpoint for order intake
- Validate order fields (symbol, price, quantity, type, etc.)
- Fetch symbol metadata (tick size, precision, leverage) from `symbol-registry-service`
- Adjust price/quantity precision
- Forward validated order to `order-service` via Redis

---

## Flow

### 1. **Receive Order (gRPC)**

- Exposes: SubmitOrder(OrderRequest)

- Accepts userId, symbol, price, qty, direction, order type, leverage, etc.

### 2. **Symbol Validation**

- Calls: symbol-registry-service → GetSymbol(symbol)

- Validates:
- Symbol exists
- Tick size
- Max leverage
- Initial/Maintenance margin
- Precision

### 3. **Order Sanitization**

- Adjusts:
- Price to allowed tick size
- Quantity precision
- Ensures:
- Price > 0 for limit orders
- Qty > 0 for all orders

### 4. **Order Dispatch**

- Publishes the final validated order to: QUEUE:ORDER

- Handled downstream by `order-service`

---

## Dependencies

- Redis (for pushing validated orders)
- `symbol-registry-service` (gRPC)
- `order-service` (listens on Redis queue)

---

## Design Features

- 🔐 Acts as a secure input boundary — no raw Redis access from client
- 🧠 Symbol metadata lookup ensures proper tick size enforcement
- 🪵 Central logging on validation failures
- 🧩 Pluggable validation rules (e.g. risk limits, daily trade caps)

---

## Production Notes

- Retry-safe Redis push
- gRPC input validation schema enforced via `.proto`
- Crash-safe with structured logging

---

## TODOs

- [ ] Add user-level risk checks (notional cap, position limits)
- [ ] Cache symbol metadata for performance
- [ ] Return granular gRPC errors (INVALID_TICK_SIZE, INVALID_MARGIN)

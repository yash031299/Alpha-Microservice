# 📦 Order Service

## Overview

The `order-service` is the **entry point for all user orders** in the Alpha OMS system. It receives new order requests (limit, market, SL), validates them, routes them to appropriate services (execution-service, match-engine), and ensures status updates are stored.

---

## Responsibilities

- Accept new order submissions via Redis Queue (`QUEUE:ORDER`)
- Parse, validate, and sanitize order requests
- Route to:
  - `match-engine` (for limit orders)
  - `execution-service` (for market & SL orders)
- Push order status updates (PENDING, PARTIAL, FILLED) to Redis
- Log and handle invalid or malformed orders gracefully

---

## Flow

### 1. **Order Intake**

- Listens on: QUEUE:ORDER

for new orders submitted by clients or order-router-service

### 2. **Validation**

- Checks:
- Required fields (userId, symbol, price, qty)
- Symbol exists (via `symbol-registry-service`)
- Tick size, precision
- Price validity
- Quantity > 0

### 3. **Routing**

- Based on `orderType`:
- `LIMIT` → Forwarded to `match-engine` (gRPC or direct call)
- `MARKET` / `SL` → Sent to `execution-service` via gRPC
- Adds metadata: reduce-only, leverage, direction, etc.

### 4. **Order Persistence**

- On `PENDING`, saves Redis entry: ORDER:{orderId}
  with status and order metadata

### 5. **Event Logging**

- Pushes updates to Redis stream or Redis hash:
- Executed qty
- Average executed price
- Remaining quantity
- Final state

---

## Redis Keys Used

- `QUEUE:ORDER` (input queue)
- `ORDER:{orderId}` (order metadata)
- Redis stream / pubsub (optional for events)

---

## Dependencies

- `symbol-registry-service` (gRPC) — for tick size, leverage validation
- `match-engine` (for limit order submission)
- `execution-service` (for market, SL orders)
- Redis for order state persistence

---

## Design Features

- 💠 Stateless (all data comes from request or Redis)
- 🧠 Clean separation of validation and execution
- ⛔ Rejects malformed/incomplete orders with meaningful logs
- ✅ Validated against symbol metadata and tick constraints

---

## TODOs

- [ ] Expose gRPC version of this service for external clients
- [ ] Implement retry logic if `match-engine` or `execution-service` fails
- [ ] Add circuit-breaker or timeout fallback

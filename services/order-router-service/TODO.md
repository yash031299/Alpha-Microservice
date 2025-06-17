# ✅ TODO.md — order-router-service

## 📦 Purpose

Route incoming orders to either `execution-service` (via gRPC) or `match-engine` (via Redis), based on type.

---

## ✅ DONE

### 🏗️ Service Architecture

- [x] Created `OrderRouterService` class with `start()`, `stop()`, `listenLoop()`, and `processOrder()`.
- [x] Removed all monolithic dependencies (`redis_update.h`, `RedisConnectionPool`, etc.).
- [x] Direct Redis integration using `hiredis`.

### 🔌 gRPC Integration

- [x] Created `ExecutionGRPCClient` with structured proto-based `sendOrderFromJson()`.
- [x] Used `order.proto` with `OrderService.SubmitOrder()` method.
- [x] JSON parsing to `OrderRequest` fields using `rapidjson`.

### ⚙️ Redis Integration

- [x] Used `BLPOP` to pull orders from `QUEUE:ORDER`.
- [x] Used `RPUSH` to push LIMIT orders to `QUEUE:MATCH:<symbol>`.

### 🔧 Build System

- [x] `meson.build` created using standalone Boost, Redis, gRPC, spdlog, protobuf.
- [x] Proto generation outputs included (`order.pb.cc`, `order.grpc.pb.cc`).
- [x] All warnings disabled via `-w`.

### 🚨 Resilience & Clean Shutdown

- [x] Added `SIGINT`/`SIGTERM` signal handling.
- [x] Catch-all exception handling in `main()`.
- [x] Clean shutdown of Redis and listener thread.

---

## 🔜 TODO (Upcoming)

### 🛡️ Fault Tolerance & Retry

- [ ] Add gRPC retry mechanism with exponential backoff (for `sendOrderFromJson()`).
- [ ] Add Redis reconnection logic on dropped connection.

### 🔍 Observability

- [ ] Integrate Prometheus metrics (orders routed, failures, latency).
- [ ] Add log trace IDs for correlation.

### 📄 Configuration & Env

- [ ] Load Redis host/port, queues, gRPC target from `.env` using `dotenv` or custom loader.
- [ ] Validate all env vars at startup.

### 🧪 Testing

- [ ] Write unit tests for `OrderRouterService::processOrder()`.
- [ ] Mock Redis & gRPC for integration tests.
- [ ] Add `meson test` target.

### 🐳 Docker

- [ ] Finalize Dockerfile with health check & small runtime image (multi-stage build).

---

## 💡 Notes

- `userManager` currently uses a stub version; full auth and RMS sync logic will be implemented in later phases.
- Redis uses direct connection now; upgrade to pooled async design if needed at scale.

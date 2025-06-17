# 📌 TODO.md — funding-engine (Updated 2025-06-11)

This document tracks the full development progress of the `funding-engine` microservice in AlphaOMS.

---

## ✅ Phase 1: Core System

- [x] `FundingCalculator`: long/short OI tracking + rate calculation
- [x] `FundingScheduler`: symbol-specific periodic trigger with retry
- [x] `FundingPublisher`: Redis stream publishing with reconnect logic
- [x] Boost Asio event loop with per-symbol timers
- [x] Meson build system + Conan + Dockerfile (multi-stage)
- [x] Graceful shutdown using signal handler
- [x] `.env`-based configuration via `ConfigLoader`

---

## 🔄 Phase 2: Real-time Integration

- [ ] **RMS gRPC client** for reading open interest dynamically
- [ ] **gRPC endpoint**: `GetFundingRate(symbol)` (optional pull API)
- [x] Redis publishing to stream: `FUNDING:STREAM:<symbol>`

---

## ✅ Phase 3: Fault Tolerance & Reliability

- [x] Exception-safe compute + publish logic
- [x] Timer recovery on failure
- [x] Redis reconnection on disconnect
- [x] Symbol isolation for thread safety
- [x] Graceful shutdown on SIGINT/SIGTERM

---

## ⚙️ Phase 4: Monitoring & Observability

- [x] HTTP liveness probe on `/healthz` (port `8081`)
- [x] In-memory metrics collector (`Metrics.hpp`)
- [ ] `/metrics` endpoint for Prometheus (future enhancement)
- [ ] Alerting on Redis connection failures (future)

---

## 🧪 Phase 5: Testing

- [ ] Unit tests for `FundingCalculator::computeFunding()`
- [ ] Integration test with mock RMS or JSON
- [ ] Load test with 1,000+ symbols

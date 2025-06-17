# 📌 TODO — symbol-registry-service (Updated: 2025-06-17)

Tracks enhancements, production readiness, and test progress for `symbol-registry-service`.

---

## ✅ Phase 1: Core Functionality

- [x] Load `symbols.json` at startup
- [x] Serve metadata via gRPC (`GetSymbolMetadata`)
- [x] Implement `GetSymbolMetadata` with locking and validation
- [x] Containerize service using Meson + Docker
- [x] Fix JSON parsing bug (corrected array vs object handling)
- [x] Fully tested via `grpcurl`
- [x] Basic Python client tested

---

## 🔄 Phase 2: Extensibility

- [ ] ✅ Add `ListAllSymbols()` RPC endpoint _(proto + impl)_
- [ ] ⏳ Support `/reload` API or file-watcher for live symbol config refresh
- [ ] ⏳ Load symbols from Redis (for dynamic symbol updates)

---

## 🔒 Phase 3: Security & Reliability

- [ ] Add TLS encryption for gRPC
- [ ] Add token/cert-based authentication
- [ ] Input validation (lowercase symbols, safe limits)

---

## 📊 Phase 4: Observability

- [ ] Prometheus metrics:
  - Total RPCs served
  - Failed lookups
  - Hot reloads triggered
- [ ] Add `/healthz` HTTP probe

---

## 🧪 Phase 5: Testing

- [ ] Unit tests:
  - `loadSymbolsFromFile()`
  - `GetSymbolMetadata()`
- [ ] Add integration tests (gRPC via Python client)
- [ ] Fuzz testing for malformed requests

---

## 🚀 Deployment Readiness

- [ ] Add `k8s/service.yaml` and `deployment.yaml`
- [ ] Benchmark with 1000+ symbols for latency
- [ ] Expose as internal endpoint for RMS, Order, etc.

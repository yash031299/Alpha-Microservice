# 🧠 TODO — Risk Service (RMS) : Dynamic Symbol Integration

## 🎯 Goal

Remove hardcoded `"BTC-USDT"` symbol and dynamically load active symbols from `symbol-registry-service` via gRPC for production-grade scalability.

---

## 📌 Tasks

### 1. Implement Symbol Loader Client (gRPC)

- [ ] Create gRPC client stub from `symbol_registry.proto`
- [ ] Create `symbol_registry_client.hpp` and `.cpp`
- [ ] Add `fetchAllSymbols()` method to request all symbols via `ListSymbols` RPC

---

### 2. Wire Symbol Loader into main.cpp

- [ ] Replace:
  ```cpp
  state.symbol = "BTC-USDT";
  ```

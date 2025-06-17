# ✅ Stream Service - TODO List (AlphaOMS)

This file tracks future enhancements, integrations, and tasks for making `stream-service` fully production-grade.

---

## 🔧 Wiring Tasks (to be done later)

- [ ] Wire published ticks to `rms-service` via Redis SUBSCRIBE
- [ ] Wire published ticks to `pnl-service` for real-time PnL updates
- [ ] Optionally broadcast ticks to `kline-service` for OHLC aggregation
- [ ] Document pub/sub contract format (tick schema)

---

## 🚀 Feature Enhancements

- [ ] Support multiple tick streams (`FUTURE:TICK:ETH-USDT`, etc.)
- [ ] Add filtering logic by symbol inside stream-service
- [ ] Use Boost Asio for non-blocking I/O (optional scale-up)
- [ ] Add retry/backoff if Redis goes down

---

## 📦 Persistence & Observability

- [ ] Add tick logging to local file for backup
- [ ] Push parsed ticks to Kafka (future scalable architecture)
- [ ] Add Prometheus metrics for ticks received/published
- [ ] Add healthcheck HTTP endpoint for Docker health probe

---

## 🧪 Testing

- [ ] Write integration test that publishes XADD and checks Redis pub
- [ ] Add Docker Compose setup with Redis + stream-service
- [ ] Write unit tests for RedisStreamListener

---

## 🗂 Cleanup

- [x] Remove `tick_parser.hpp/.cpp`
- [x] Remove unused `config.hpp/.cpp`
- [x] Replace `RedisSubscriber` with `RedisStreamListener`

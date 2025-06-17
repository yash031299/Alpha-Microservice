# 📌 TODO — kline-service

This service aggregates trades into OHLCV candles per symbol and interval, and publishes them to Redis and audit-log-service.

---

## ✅ Phase 1: Core System

- [x] `TradeTick` and `Candle` structs (`1m`, `5m`, `1h`)
- [x] In-memory `CandleAggregator` with thread safety
- [x] `KlineScheduler` with async flush timer
- [x] Boost Asio event loop
- [x] Clean `meson.build` with warning suppression
- [x] Dockerfile with multi-stage build and runtime setup

---

## ✅ Phase 2: Input/Output Integration

- [x] Subscribe to trade feed from `match-engine` via Redis Stream (`TRADE_STREAM:<symbol>`)
- [x] Publish candles to Redis Pub/Sub (`ohlcv-stream:<symbol>`)
- [ ] Push candle logs to `audit-log-service` using gRPC
- [ ] Push candles to PostgreSQL/ClickHouse (optional, deferred until DB integration phase)

---

## ✅ Phase 3: Stability and Scaling

- [x] Full try-catch fault isolation (Redis, gRPC, scheduler)
- [ ] Cap candle history in memory (optional LRU cache; not needed for streaming charting)
- [ ] Deduplicate trades per timestamp (not critical with reliable Redis stream)
- [ ] Memory profiling or flush benchmarking (to be done during load testing)

---

## ✅ Phase 4: Observability

- [x] `/healthz` HTTP endpoint for liveness checks
- [x] `/metrics` Prometheus-style endpoint
  - `kline_flush_duration`
  - `kline_ohlcv_count`

---

## 🧪 Phase 5: Testing

- [ ] Unit test `CandleAggregator::addTrade()` and verify OHLCV output
- [ ] Integration test flush cycle with simulated trades
- [ ] Load test with 10k trades/sec on 1000 symbols (to be scheduled post-integration)

---

## 🧠 Notes

- Service is horizontally scalable (can shard by symbol across instances)
- Real-time candles served via Redis Pub/Sub
- Candle logs persisted via gRPC to audit-log-service
- DB write and LRU cache are optional depending on product needs

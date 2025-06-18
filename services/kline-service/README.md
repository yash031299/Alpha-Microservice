# 📈 Kline Service

## Overview
The `kline-service` aggregates trades into **OHLCV candles** (Open, High, Low, Close, Volume) over specific time intervals. It powers historical charting and real-time analytics by transforming raw trade data into structured candlestick bars.

---

## Responsibilities

- Listen to Redis Stream `STREAM:TRADE`
- Aggregate trades into time-bucketed OHLCV format
- Periodically flush completed candles to Redis or storage
- Maintain in-memory cache of current partial candle

---

## Redis Integration

- Listens on:
  ```
  STREAM:TRADE
  ```
- Each trade entry:
  ```json
  {
    "user": "test-user-1",
    "symbol": "BTC-USDT",
    "price": 30000.0,
    "qty": 0.5,
    "side": "BUY"
  }
  ```

---

## Aggregation Logic

1. Round trade timestamp to nearest interval (e.g., 1m)
2. Update:
   - `Open`: first trade price of interval
   - `High`: max price so far
   - `Low`: min price so far
   - `Close`: latest price
   - `Volume`: sum of trade quantities
3. At interval end, flush candle and reset

---

## Output Format

Example 1-minute candle:
```json
{
  "timestamp": "2025-06-18T12:30:00Z",
  "open": 30100.0,
  "high": 30250.0,
  "low": 30080.0,
  "close": 30200.0,
  "volume": 12.5
}
```

---

## Consumers

- Chart UI (via REST/WebSocket)
- Analytics engines
- Snapshot backup
- Backtest framework

---

## Features

- 🕒 Configurable time intervals (1m, 5m, 1h)
- 🧠 Smart bucket caching for performance
- 🔄 Graceful flush on shutdown
- 🔒 Thread-safe data aggregation

---

## TODOs

- [ ] Store candles to persistent DB (e.g., PostgreSQL, ClickHouse)
- [ ] Expose gRPC endpoint to fetch OHLCV history
- [ ] Add resampling for multi-timeframe support
- [ ] Metrics: candle lag, flush duration


# ✅ TODO — user-service

Tracks enhancements for scalability, observability, and fault-tolerance

---

## Phase 1: Core Functionality (Completed)

- [x] Load configuration from `.env` file (Redis host/port, gRPC port, flush interval)
- [x] Wrap all critical code sections with `try/catch` for crash resilience
- [x] Implement RedisSafeClient with retry and safe command execution
- [x] Refactor UserWalletStore and UserMetadataStore with fault tolerance
- [x] Implement MarginEventListener with reconnect and exception safety
- [x] Implement gRPC UserService with methods:
  - `UpdateWallet`
  - `GetWallet`
  - `GetUserMetadata`
  - All handlers wrapped in try/catch and logging

---

## Phase 2: Observability and Metrics

- [ ] Add Prometheus metrics for:
  - Wallet flush counts
  - Margin updates processed
  - Redis connection health
- [ ] Expose `/metrics` HTTP endpoint for scraping

---

## Phase 3: Resilience and Retry

- [ ] Add caching of margin updates if Redis is temporarily down
- [ ] Implement advanced Redis client async features or use Boost Asio for improved performance
- [ ] Enhance retry/backoff strategies in RedisSafeClient

---

## Phase 4: Scalability

- [ ] Replace global mutexes with lock striping or sharded maps to reduce contention
- [ ] Support horizontal scaling of multiple user-service instances behind gRPC load balancer
- [ ] Optimize Redis data access patterns and memory footprint

---

## Phase 5: Security and Validation

- [ ] Add input validation and sanitization for gRPC requests
- [ ] Implement authentication and authorization for gRPC APIs
- [ ] Add logging and alerting for suspicious activities or failures

---

### Notes

- Current version is stable and handles basic wallet and metadata operations with margin update subscription
- Next focus should be on observability and resilience before scale-out

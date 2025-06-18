# ⚙️ Config Service

## Overview

The `config-service` acts as a centralized provider for **application configuration** across microservices. It serves `.env` variables and JSON/YAML config files via gRPC to allow dynamic config updates and centralized access.

---

## Responsibilities

- Load configuration from `.env` and static config files (e.g., `config.json`)
- Serve config values via gRPC
- Provide reload-on-change (future)
- Support overrides and fallbacks

---

## Startup Behavior

- Parses `.env` using dotenv parser
- Parses additional JSON config if available
- Combines and exposes all config values

---

## gRPC Interface

### RPC

```proto
rpc GetConfig (ConfigRequest) returns (ConfigResponse);
```

### Response

```json
{
  "env": {
    "REDIS_HOST": "localhost",
    "GRPC_PORT": "50051"
  },
  "json_config": {
    "risk_interval_ms": 1000
  }
}
```

---

## Consumers

- All services that need dynamic `.env` or system-wide settings
- `risk-service`, `wallet-sync-service`, `execution-service`, etc.

---

## Features

- 📦 Unified configuration hub
- 🧵 Thread-safe in-memory config storage
- 🔁 Live reload-ready (future)
- 🔐 Prevents scattered `.env` misuse

---

## TODOs

- [ ] Add file watcher for live reload
- [ ] Add support for secrets (Vault, AWS Secrets Manager)
- [ ] Add config versioning or change history
- [ ] Support config validation schemas

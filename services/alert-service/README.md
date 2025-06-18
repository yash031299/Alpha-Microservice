# 🚨 Alert Service

## Overview
The `alert-service` is responsible for **generating and dispatching system alerts** across the OMS. It listens to critical triggers like margin violations, funding anomalies, trade errors, or downtime signals, and sends notifications to operators, dashboards, or messaging tools.

---

## Responsibilities

- Receive alert triggers via gRPC or Redis queue
- Classify alerts based on severity (INFO, WARN, ERROR, FATAL)
- Dispatch alerts via:
  - Logs
  - Email (future)
  - Webhooks (e.g., Slack, PagerDuty)
- Track alert history and de-duplicate repeat notifications

---

## Triggers

- Manual gRPC trigger:
  ```proto
  rpc SendAlert (AlertRequest) returns (AlertAck);
  ```

- Redis queue (optional):
  ```
  QUEUE:ALERT
  ```

### Example Alert
```json
{
  "type": "margin_violation",
  "user_id": "user123",
  "severity": "ERROR",
  "message": "User equity below maintenance margin",
  "timestamp": "2025-06-18T12:34:56Z"
}
```

---

## Flow

1. Trigger received from RMS, liquidation, or risk-service
2. Alert validated and classified
3. Sent to:
   - Logs (file, syslog)
   - External webhook or dashboard
4. Future: Alert state persisted for audit and UI

---

## Consumers

- Admin dashboard
- Slack or email endpoints
- Alert database or audit-log-service

---

## Features

- ⚠️ Multi-level severity (INFO → FATAL)
- 📬 Pluggable dispatch mechanism
- 🧠 De-duplication for flood control
- 🧵 Async dispatch with retry on failure

---

## TODOs

- [ ] Add persistent alert history with TTL
- [ ] Add Slack, SMTP, and PagerDuty integrations
- [ ] Add webhook retry policy
- [ ] Create admin UI panel for alert control

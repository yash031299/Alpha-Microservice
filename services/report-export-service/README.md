# 🧾 Report Export Service

## Overview
The `report-export-service` generates **user-specific reports** in PDF or CSV format. These reports include trading history, PnL statements, and ledger entries. It's a utility service often used for regulatory exports, user dashboards, and email digests.

---

## Responsibilities

- Accept export requests via gRPC
- Format input JSON into ledger-style report
- Render report into:
  - CSV
  - PDF (uses wkhtmltopdf or equivalent)
- Store or return file to client

---

## gRPC Interface

### RPC
```proto
rpc ExportUserReport (ExportRequest) returns (ExportResponse);
```

### Example Request Payload
```json
{
  "user_id": "test-user-1",
  "type": "ledger",
  "format": "pdf",
  "json_data": "[{"timestamp":"2025-06-16 22:55:00","type":"realized_pnl","symbol":"BTC-USDT","amount":300.25}]"
}
```

### Response
- If `format = pdf`: Returns `base64`-encoded string
- If `format = csv`: Returns inline CSV string or download path

---

## Report Types

- Ledger History (funding, trades, PnL)
- Monthly/weekly summaries
- Export for Tax/Compliance teams

---

## Features

- 📄 PDF & CSV output
- 🧠 Format-agnostic rendering engine
- 📦 Compressible output (zip)
- ⏱️ Timestamped reports

---

## Dependencies

- PDF: wkhtmltopdf (or a compatible renderer)
- JSON parser (nlohmann/json or rapidjson)
- CSV: in-memory stringstream

---

## TODOs

- [ ] Replace wkhtmltopdf (deprecated) with headless Chromium
- [ ] Add email dispatch of report
- [ ] Add Prometheus export metrics
- [ ] Move templates to customizable markdown or HTML files


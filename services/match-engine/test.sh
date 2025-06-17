#!/bin/bash

PROTO_PATH="proto/match.proto"
TARGET="localhost:50051"

echo "🟢 Submitting LIMIT BUY order..."
grpcurl -plaintext -proto $PROTO_PATH -d '{
  "orderId": "order-001",
  "userId": "user-1",
  "symbol": "BTC-USDT",
  "price": 31000,
  "quantity": 0.5,
  "direction": "BUY",
  "ordertype": "LIMIT"
}' $TARGET match.MatchEngine/SubmitOrder
echo ""

echo "🟢 Submitting MARKET SELL order..."
grpcurl -plaintext -proto $PROTO_PATH -d '{
  "orderId": "order-002",
  "userId": "user-2",
  "symbol": "BTC-USDT",
  "quantity": 0.3,
  "direction": "SELL",
  "ordertype": "MARKET"
}' $TARGET match.MatchEngine/SubmitOrder
echo ""

echo "🟡 Modifying LIMIT order..."
grpcurl -plaintext -proto $PROTO_PATH -d '{
  "orderId": "order-001",
  "userId": "user-1",
  "symbol": "BTC-USDT",
  "price": 31200,
  "quantity": 0.4
}' $TARGET match.MatchEngine/ModifyOrder
echo ""

echo "🔴 Cancelling LIMIT order..."
grpcurl -plaintext -proto $PROTO_PATH -d '{
  "orderId": "order-001",
  "symbol": "BTC-USDT"
}' $TARGET match.MatchEngine/CancelOrder
echo ""

echo "📦 Checking Redis state (ORDER:order-001)..."
redis-cli HGETALL ORDER:order-001

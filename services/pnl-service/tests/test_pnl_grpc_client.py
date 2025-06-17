import grpc
import pnl_pb2
import pnl_pb2_grpc

def run():
    channel = grpc.insecure_channel('localhost:50052')
    stub = pnl_pb2_grpc.PnLServiceStub(channel)

    print("Sending test trade...")

    response = stub.SendTrade(pnl_pb2.TradeUpdate(
        user_id="test-user-99",
        symbol="BTC-USDT",
        price=30000.0,
        quantity=1.0,
        side="BUY"
    ))

    print("✅ Trade sent successfully.")

if __name__ == '__main__':
    run()

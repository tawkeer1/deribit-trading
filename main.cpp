#include "DeribitClient.hpp"
#include "DeribitWSClient.hpp"
#include "WSBroadcastServer.hpp"

int main() {
    DeribitClient client("KDdbgieA", "rGZW5VaVsFJ1EzJtjlZRX3pH911W3YMUsW6sI2RXOxk");

    if (!client.authenticate()) {
        cerr << "Auth failed. Exiting.\n";
        return 1;
    }

    client.placeOrder("BTC-PERPETUAL", "50000", "10");
    // client.cancelOrder("order_id_here");
    // client.modifyOrder("order_id_here", 30, 25000.0);
    // client.getOrderBook("BTC-PERPETUAL");
    // client.getOpenOrders();
    // client.getPosition("BTC-PERPETUAL");

    // WebSocket
    
    WSBroadcastServer server;
    
    DeribitWSClient deribitWS([&server](const string& msg) {
        server.broadcast(msg);  // Forward Deribit data to all connected clients
    },"ws://localhost:9002");
    
    server.run(9003); 
    deribitWS.run();  // Start Deribit WebSocket client

    cout << "Server running. Visit ws://localhost:9002 from a client.\n";

    while (true) this_thread::sleep_for(chrono::seconds(10));  // Keep alive

    return 0;
}

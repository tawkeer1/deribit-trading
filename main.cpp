#include "DeribitClient.hpp"
#include "DeribitWSClient.hpp"
#include "WSBroadcastServer.hpp"
#include<bits/stdc++.h>
using namespace std;
int main() {
    DeribitClient client("KDdbgieA", "rGZW5VaVsFJ1EzJtjlZRX3pH911W3YMUsW6sI2RXOxk");

    if (!client.authenticate()) {
        cerr << "Auth failed. Exiting.\n";
        return 1;
    }
    int choice;
    cout<<"Enter 1 to manage order(place,cancel,getorderbook etc) else enter 2 for getting realtime data from deribit\n";
    cin>>choice;
    if(choice == 1){
        cout<<"Enter the order type you want to place\n";
        cout<<"1. Place Order\n2. CancelOrder\n3. Modify Order\n4. Get OrderBook\n5. Get Position\n6. Get Open Orders\n";
        int innerChoice;
        cin >> innerChoice;
        switch (innerChoice) {
        case 1: {
        string instrument, price, amount;
        cout << "Enter the instrument name\n";
        cin >> instrument;
        cout << "Enter the price\n";
        cin >> price;
        cout << "Enter the amount\n";
        cin >> amount;
        client.placeOrder(instrument, price, amount);
        break;
    }
    case 2: {
        string orderId;
        cout << "Enter the order id to cancel\n";
        cin >> orderId;
        client.cancelOrder(orderId);
        break;
    }
    case 3: {
        string orderIdToModify;
        int newAmount;
        double newPrice;
        cout << "Enter the order id to modify\n";
        cin >> orderIdToModify;
        cout << "Enter the new amount\n";
        cin >> newAmount;
        cout << "Enter the new price\n";
        cin >> newPrice;
        client.modifyOrder(orderIdToModify, newAmount, newPrice);
        break;
    }
    case 4: {
        string instrumentForOrderBook;
        cout << "Enter the instrument name for order book\n";
        cin >> instrumentForOrderBook;
        client.getOrderBook(instrumentForOrderBook);
        break;
    }
    case 5: {
        string instrumentForPosition;
        cout << "Enter the instrument name for position\n";
        cin >> instrumentForPosition;
        client.getPosition(instrumentForPosition);
        break;
    }
    case 6: {
        client.getOpenOrders();
        break;
    }
    default: {
        cout << "Invalid choice\n";
        break;
    }
}


    }
    else{
        cout<<"You have selected to get real time data from deribit. Make sure server is running\n";
        WSBroadcastServer server;
    
      DeribitWSClient deribitWS([&server](const string& msg) {
        server.broadcast(msg);  // Forward Deribit data to all connected clients
    },"ws://localhost:9002");
    
    server.run(9003); 
    deribitWS.run();  // Start Deribit WebSocket client

    cout << "Server running. Visit ws://localhost:9002 from a client.\n";

    while (true) this_thread::sleep_for(chrono::seconds(10));  // Keep alive
    }
    
    return 0;
}

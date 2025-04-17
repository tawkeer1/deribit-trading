#define ASIO_STANDALONE
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <functional>
#include <thread>
#include <iostream>

using namespace std;
using json = nlohmann::json;

typedef websocketpp::client<websocketpp::config::asio_client> DeribitWebSocketClient;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;

class DeribitWSClient {
private:
    DeribitWebSocketClient wsClient;
    websocketpp::connection_hdl conn_hdl;
    string uri;
    function<void(const string&)> messageHandler;

public:
    DeribitWSClient(function<void(const string&)> handler,
                    const string& endpoint = "ws://localhost:9002")
        : uri(endpoint), messageHandler(handler) {
        
        wsClient.clear_access_channels(websocketpp::log::alevel::all);
        wsClient.clear_error_channels(websocketpp::log::elevel::all);
        wsClient.init_asio();

        wsClient.set_open_handler([this](websocketpp::connection_hdl hdl) {
            cout << "Connected to proxy!" << endl;
            conn_hdl = hdl;
        
            thread([this, hdl]() {
                this_thread::sleep_for(chrono::milliseconds(1000)); // Wait 1s
                json payload = {
                    {"jsonrpc", "2.0"},
                    {"method", "public/subscribe"},
                    {"id", 42},
                    {"params", {
                        {"channels", {
                            "book.BTC-PERPETUAL.100ms",
                            "book.ETH-PERPETUAL.100ms"
                        }}
                    }}
                };
        
                cout << "Sending subscription payload: " << payload.dump() << endl;
                wsClient.send(hdl, payload.dump(), websocketpp::frame::opcode::text);
                cout << "Sent subscription payload." << endl;
            }).detach(); // fire-and-forget thread
        });
        
        
        
        wsClient.set_message_handler([this](websocketpp::connection_hdl, message_ptr msg) {
            if (messageHandler) {
                cout<<"Received message: " << msg->get_payload() << endl;
                messageHandler(msg->get_payload());
            }
        });
        cout<<"Setting message handler."<<endl;
        wsClient.set_fail_handler([](websocketpp::connection_hdl) {
            cerr << "WebSocket connection failed." << endl;
        });
    }

    void run() {
        websocketpp::lib::error_code ec;
        auto con = wsClient.get_connection(uri, ec);
        if (ec) {
            cerr << "Connection error: " << ec.message() << endl;
            return;
        }

        wsClient.connect(con);
        thread([this]() { wsClient.run(); }).detach();
    }
};

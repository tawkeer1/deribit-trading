#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>
using namespace std;
typedef websocketpp::server<websocketpp::config::asio> BroadcastServer;


class WSBroadcastServer {
private:
    BroadcastServer server;
    set<websocketpp::connection_hdl, owner_less<websocketpp::connection_hdl>> connections;
    mutex connMutex;

public:
    WSBroadcastServer() {
        server.init_asio();

        server.set_open_handler([this](websocketpp::connection_hdl hdl) {
            lock_guard<mutex> lock(connMutex);
            connections.insert(hdl);
        });

        server.set_close_handler([this](websocketpp::connection_hdl hdl) {
            lock_guard<mutex> lock(connMutex);
            connections.erase(hdl);
        });

        server.set_message_handler([](websocketpp::connection_hdl, BroadcastServer::message_ptr msg) {
            // You could parse client subscription requests here
            cout << "Client Message: " << msg->get_payload() << endl;
        });
    }

    void run(uint16_t port) {
        server.listen(port);
        server.start_accept();
        thread([this]() { server.run(); }).detach();
    }

    void broadcast(const string& message) {
        lock_guard<mutex> lock(connMutex);
        for (auto it = connections.begin(); it != connections.end(); ) {
            websocketpp::lib::error_code ec;
            server.send(*it, message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                cerr << "Send error: " << ec.message() << endl;
                it = connections.erase(it);  // Remove bad connection
            } else {
                ++it;
            }
        }
    }
    
};

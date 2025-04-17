#pragma once

#include <iostream>
#include <string>
#include <curl/curl.h>
#include "include/json.hpp"
using namespace std;
using json = nlohmann::json;

class DeribitClient {
private:
    string clientId;
    string clientSecret;
    string accessToken;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    string sendRequest(const string& url, const json& payload, const string& token = "") {
        string readBuffer;
        CURL* curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            string jsonStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            if (!token.empty()) {
                headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            //actually sends the http request and receives res
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        return readBuffer;
    }

public:
    DeribitClient(const string& id, const string& secret)
        : clientId(id), clientSecret(secret) {}

    bool authenticate() {
        json payload = {
            {"id", 0},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"},
                {"client_id", clientId},
                {"client_secret", clientSecret}
            }},
            {"jsonrpc", "2.0"}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
        auto resJson = json::parse(response);

        if (resJson.contains("result") && resJson["result"].contains("access_token")) {
            accessToken = resJson["result"]["access_token"];
            cout << "Authenticated successfully.\n";
            return true;
        } else {
            cerr << "Failed to authenticate.\n";
            return false;
        }
    }

    void placeOrder(const string& instrument, const string& price, const string& amount) {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument},
                {"type", "limit"},
                {"price", price},
                {"amount", amount}
            }},
            {"id", 1}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/private/buy", payload, accessToken);
        cout << "Placed Order Response: " << response << "\n";
    }

    void cancelOrder(const string& orderId) {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/cancel"},
            {"params", {{"order_id", orderId}}},
            {"id", 2}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload, accessToken);
        cout << "Cancel Order Response: " << response << "\n";
    }

    void modifyOrder(const string& orderId, int amount, double price) {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/edit"},
            {"params", {
                {"order_id", orderId},
                {"amount", amount},
                {"price", price}
            }},
            {"id", 3}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/private/edit", payload, accessToken);
        cout << "Modify Order Response: " << response << "\n";
    }

    void getOrderBook(const string& instrument) {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "public/get_order_book"},
            {"params", {{"instrument_name", instrument}}},
            {"id", 4}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload);
        auto resJson = json::parse(response);

        cout << "Order Book for " << instrument << ":\n"
                  << "Best Bid: " << resJson["result"]["best_bid_price"]
                  << " | Ask: " << resJson["result"]["best_ask_price"] << "\n";
    }

    void getPosition(const string& instrument) {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_position"},
            {"params", {{"instrument_name", instrument}}},
            {"id", 5}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);
        cout << "Position Response: " << response << "\n";
    }

    void getOpenOrders() {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_open_orders"},
            {"params", {{"kind", "future"}, {"type", "limit"}}},
            {"id", 6}
        };

        string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders", payload, accessToken);
        cout << "Open Orders: " << response << "\n";
    }

    string getToken() const {
        return accessToken;
    }
};

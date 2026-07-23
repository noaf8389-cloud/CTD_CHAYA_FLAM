#pragma once

#include <functional>
#include <memory>
#include <string>

namespace ix { class WebSocket; }

class ServerConnection {
public:
    // Creates the socket and sets its target URL, but does not connect yet.
    explicit ServerConnection(const std::string& server_url);
    // Closes the WebSocket connection.
    ~ServerConnection();

    // Opens the connection. Call only after setOnMessage, so no early messages are missed.
    void start();

    // Sends a raw text message to the server.
    void send(const std::string& message);

    // Registers a handler to be called with each raw text message received from the server.
    void setOnMessage(std::function<void(const std::string&)> handler);

private:
    std::unique_ptr<ix::WebSocket> socket_;
};

#pragma once

#include <ixwebsocket/IXWebSocketServer.h>

#include "command_handler.hpp"
#include "network_publisher.hpp"

class GameWebSocketServer {
public:
    /// Creates and configures the WebSocket server.
    GameWebSocketServer(int port, NetworkPublisher& publisher, CommandHandler& commandHandler);

    /// Starts listening for incoming WebSocket connections.
    bool start();
    /// Blocks until the WebSocket server stops.
    void wait();

private:
    ix::WebSocketServer server_;
    NetworkPublisher& publisher_;
    CommandHandler& commandHandler_;
};

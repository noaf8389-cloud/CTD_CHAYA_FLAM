#pragma once

#include <ixwebsocket/IXWebSocketServer.h>

#include "command_handler.hpp"
#include "network_publisher.hpp"
#include "../player_registry.hpp"

class GameWebSocketServer {
public:
    /// Creates and configures the WebSocket server.
    GameWebSocketServer(int port, NetworkPublisher& publisher, CommandHandler& commandHandler, PlayerRegistry& playerRegistry);

    /// Starts listening for incoming WebSocket connections.
    bool start();
    /// Blocks until the WebSocket server stops.
    void wait();

private:
    ix::WebSocketServer server_;
    NetworkPublisher& publisher_;
    CommandHandler& commandHandler_;
    PlayerRegistry& playerRegistry_;
};

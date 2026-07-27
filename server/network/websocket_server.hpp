#pragma once

#include <ixwebsocket/IXWebSocketServer.h>

#include "command_handler.hpp"
#include "network_publisher.hpp"
#include "../player_registry.hpp"
#include "../db/player_account_store.hpp"

class GameWebSocketServer {
public:
    /// Creates and configures the WebSocket server.
    GameWebSocketServer(int port, NetworkPublisher& publisher, CommandHandler& commandHandler, PlayerRegistry& playerRegistry, PlayerAccountStore& accounts);

    /// Starts listening for incoming WebSocket connections.
    bool start();
    /// Blocks until the WebSocket server stops.
    void wait();

private:
    ix::WebSocketServer server_;
    NetworkPublisher& publisher_;
    CommandHandler& commandHandler_;
    PlayerRegistry& playerRegistry_;
    PlayerAccountStore& accounts_;
};

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include <ixwebsocket/IXWebSocketServer.h>

#include "../lobby/lobby_registry.hpp"
#include "../lobby/matchmaker.hpp"
#include "../matches/game_registry.hpp"
#include "../db/player_account_store.hpp"
#include "../player_registry.hpp"

class GameMatch;
struct LoginRequest;

class GameWebSocketServer {
public:
    /// Creates and configures the WebSocket server.
    GameWebSocketServer(int port, LobbyRegistry& lobbyRegistry, GameRegistry& gameRegistry, Matchmaker& matchmaker, PlayerAccountStore& accounts);

    /// Starts listening for incoming WebSocket connections.
    bool start();
    /// Blocks until the WebSocket server stops.
    void wait();

    /// Attaches any player Matchmaker has just paired to their new match, so they start receiving that match's broadcasts.
    /// Call once per server tick, right. after Matchmaker::tick().
    void attachNewlyMatchedConnections();

private:
    struct ConnectionState {
        LobbyRegistry::ConnectionId lobbyId;
        std::shared_ptr<ix::WebSocket> socket;
        std::optional<std::string> username;
        GameMatch* match = nullptr;
        PlayerId matchPlayerId = 0;
    };
    
    void handleLogin(ix::WebSocket* rawConnection, ConnectionState& state, const LoginRequest& request);
    void handleGameMessage(ConnectionState& state, const std::string& rawMessage);
    void handleClose(ix::WebSocket* rawConnection, ConnectionState& state);
    void attachToMatch(ConnectionState& state, GameMatch* match, bool isResume);

    ix::WebSocketServer server_;
    LobbyRegistry& lobbyRegistry_;
    GameRegistry& gameRegistry_;
    Matchmaker& matchmaker_;
    PlayerAccountStore& accounts_;

    std::mutex connectionsMutex_;
    std::unordered_map<ix::WebSocket*, ConnectionState> connections_;
    std::unordered_map<std::string, ix::WebSocket*> waitingByUsername_;};

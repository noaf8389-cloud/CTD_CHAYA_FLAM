#include "websocket_server.hpp"

#include <iostream>

#include "login_message.hpp"
#include "../matches/game_match.hpp"
#include "../third_party/nlohmann/json.hpp"
#include "logging/logger.hpp"

using json = nlohmann::json;

namespace {
    constexpr long long kDisconnectGraceMs = 20000;
}

GameWebSocketServer::GameWebSocketServer(int port, LobbyRegistry& lobbyRegistry, GameRegistry& gameRegistry,
                                          Matchmaker& matchmaker, PlayerAccountStore& accounts)
    : server_(port, "0.0.0.0"), lobbyRegistry_(lobbyRegistry), gameRegistry_(gameRegistry),
      matchmaker_(matchmaker), accounts_(accounts) {
    matchmaker_.setOnNoMatchFound([this](const std::string& username) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        auto it = waitingByUsername_.find(username);
        if (it == waitingByUsername_.end()) return;

        json response;
        response["type"] = "NoMatchFoundEvent";
        it->second->send(response.dump());
        waitingByUsername_.erase(it);
        Logger::info("No match found for username=" + username);
    });

    server_.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
               std::shared_ptr<ix::ConnectionState> /*connectionState*/) {
            auto webSocket = weakWebSocket.lock();
            if (!webSocket) {
                return;
            }

            ix::WebSocket* rawConnection = webSocket.get();
            LobbyRegistry::ConnectionId lobbyId = lobbyRegistry_.registerConnection();
            Logger::info("Client connected (lobbyId=" + std::to_string(lobbyId) + ")");

            {
                std::lock_guard<std::mutex> lock(connectionsMutex_);
                connections_[rawConnection] = ConnectionState{lobbyId, webSocket, std::nullopt, nullptr, 0};
            }

            webSocket->setOnMessageCallback(
                [this, rawConnection](const ix::WebSocketMessagePtr& message) {
                    if (message->type == ix::WebSocketMessageType::Message) {
                        std::lock_guard<std::mutex> lock(connectionsMutex_);
                        auto it = connections_.find(rawConnection);
                        if (it == connections_.end()) return;
                        ConnectionState& state = it->second;

                        if (auto request = parseLoginRequest(message->str)) {
                            handleLogin(rawConnection, state, *request);
                            return;
                        }

                        if (state.match != nullptr) {
                            try {
                                handleGameMessage(state, message->str);
                            } catch (const std::exception& e) {
                                Logger::error("Exception while handling message \"" + message->str + "\": " + e.what());
                            }
                        }
                    } else if (message->type == ix::WebSocketMessageType::Close) {
                        std::lock_guard<std::mutex> lock(connectionsMutex_);
                        auto it = connections_.find(rawConnection);
                        if (it == connections_.end()) return;
                        handleClose(rawConnection, it->second);
                        connections_.erase(it);
                    }
                });
        });
}

void GameWebSocketServer::handleLogin(ix::WebSocket* rawConnection, ConnectionState& state, const LoginRequest& request) {
    LoginResult result = accounts_.loginOrRegister(request.username, request.password);
    Logger::info("Login attempt: username=" + request.username + " success=" + (result.success ? "true" : "false"));

    if (result.success) {
        lobbyRegistry_.identify(state.lobbyId, request.username, result.rating);
        state.username = request.username;

        GameMatch* existingMatch = gameRegistry_.matchFor(request.username);
        if (existingMatch != nullptr) {
            attachToMatch(state, existingMatch, /*isResume=*/true);
            Logger::info("Session resumed: username=" + request.username);
        } else {
            matchmaker_.enqueue(request.username, result.rating);
            waitingByUsername_[request.username] = rawConnection;
            Logger::info("Enqueued for matchmaking: username=" + request.username + " rating=" + std::to_string(result.rating));
        }
    }

    json response;
    response["type"] = "LoginResultEvent";
    response["payload"] = {{"success", result.success}, {"rating", result.rating}};
    state.socket->send(response.dump());
}

void GameWebSocketServer::attachToMatch(ConnectionState& state, GameMatch* match, bool isResume) {
    PlayerId matchPlayerId = match->players().registerConnection();
    if (isResume) {
        match->players().tryResumeSession(matchPlayerId, *state.username);
    } else {
        match->players().setUsername(matchPlayerId, *state.username);
    }
    match->addConnection(state.socket);

    state.match = match;
    state.matchPlayerId = matchPlayerId;

    Logger::info(std::string(isResume ? "Player resumed match: username=" : "Player joined new match: username=") + *state.username);
}

void GameWebSocketServer::handleGameMessage(ConnectionState& state, const std::string& rawMessage) {
    std::optional<char> color = state.match->players().colorFor(state.matchPlayerId);
    if (!color.has_value()) return;
    state.match->handleMessage(rawMessage, *color);
}

void GameWebSocketServer::handleClose(ix::WebSocket* rawConnection, ConnectionState& state) {
    if (state.match != nullptr) {
        std::optional<char> color = state.match->players().colorFor(state.matchPlayerId);
        if (color.has_value()) {
            Logger::info("Client disconnected, entering " + std::to_string(kDisconnectGraceMs) +
                         "ms grace (username=" + *state.username + ")");
            state.match->players().beginDisconnectGrace(state.matchPlayerId, kDisconnectGraceMs);
        }
        state.match->removeConnection(rawConnection);
    } else if (state.username.has_value()) {
        matchmaker_.dequeue(*state.username);
        waitingByUsername_.erase(*state.username);
        Logger::info("Client disconnected while waiting for a match (username=" + *state.username + ")");
    } else {
        Logger::info("Client disconnected (lobbyId=" + std::to_string(state.lobbyId) + ")");
    }

    lobbyRegistry_.unregisterConnection(state.lobbyId);
}

void GameWebSocketServer::attachNewlyMatchedConnections() {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto it = waitingByUsername_.begin(); it != waitingByUsername_.end(); ) {
        GameMatch* match = gameRegistry_.matchFor(it->first);
        if (match != nullptr) {
            auto connIt = connections_.find(it->second);
            if (connIt != connections_.end()) {
                attachToMatch(connIt->second, match, /*isResume=*/false);
            }
            it = waitingByUsername_.erase(it);
        } else {
            ++it;
        }
    }
}

bool GameWebSocketServer::start() {
    auto result = server_.listen();
    if (!result.first) {
        std::cerr << "WebSocket server failed to listen: " << result.second << std::endl;
        Logger::error("WebSocket server failed to listen: " + result.second);
        return false;
    }
    server_.start();
    return true;
}

void GameWebSocketServer::wait() {
    server_.wait();
}

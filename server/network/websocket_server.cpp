#include "login_message.hpp"
#include <iostream>
#include "websocket_server.hpp"
#include "../third_party/nlohmann/json.hpp"
#include "logging/logger.hpp"

using json = nlohmann::json;

GameWebSocketServer::GameWebSocketServer(int port, NetworkPublisher& publisher, CommandHandler& commandHandler, PlayerRegistry& playerRegistry, PlayerAccountStore& accounts)
    : server_(port, "0.0.0.0"), publisher_(publisher), commandHandler_(commandHandler), playerRegistry_(playerRegistry), accounts_(accounts) {
    server_.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
               std::shared_ptr<ix::ConnectionState> /*connectionState*/) {
            auto webSocket = weakWebSocket.lock();
            if (!webSocket) {
                return;
            }

            ix::WebSocket* rawConnection = webSocket.get();
            PlayerId playerId = playerRegistry_.registerConnection();
            Logger::info("Client connected (playerId=" + std::to_string(playerId) + ")");

            webSocket->setOnMessageCallback(
                [this, webSocket, rawConnection, playerId](const ix::WebSocketMessagePtr& message) {
                    if (message->type == ix::WebSocketMessageType::Open) {
                        publisher_.addConnection(webSocket);
                    } else if (message->type == ix::WebSocketMessageType::Message) {
                        if (auto request = parseLoginRequest(message->str)) {
                            LoginResult result = accounts_.loginOrRegister(request->username, request->password);
                            Logger::info("Login attempt: username=" + request->username +
                                         " success=" + (result.success ? "true" : "false"));
                            if (result.success) {
                                playerRegistry_.setUsername(playerId, request->username);
                            }
                            json response;
                            response["type"] = "LoginResultEvent";
                            response["payload"] = {{"success", result.success}, {"rating", result.rating}};
                            webSocket->send(response.dump());
                            return;
                        }

                        std::optional<char> color = playerRegistry_.colorFor(playerId);
                        if (!color.has_value()) {
                            return;
                        }
                        try {
                            commandHandler_.handleMessage(message->str, *color);
                        } catch (const std::exception& e) {
                            std::cerr << "Exception while handling message \"" << message->str << "\": " << e.what() << std::endl;
                            Logger::error("Exception while handling message \"" + message->str + "\": " + e.what());
                        }
                    } else if (message->type == ix::WebSocketMessageType::Close) {
                        Logger::info("Client disconnected (playerId=" + std::to_string(playerId) + ")");
                        playerRegistry_.unregisterConnection(playerId);
                        publisher_.removeConnection(rawConnection);
                    }
                });
        });
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

#include "websocket_server.hpp"

#include <iostream>

GameWebSocketServer::GameWebSocketServer(int port, NetworkPublisher& publisher, CommandHandler& commandHandler)
    : server_(port, "0.0.0.0"), publisher_(publisher), commandHandler_(commandHandler) {
    server_.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> weakWebSocket,
               std::shared_ptr<ix::ConnectionState> /*connectionState*/) {
            auto webSocket = weakWebSocket.lock();
            if (!webSocket) {
                return;
            }

            ix::WebSocket* rawConnection = webSocket.get();

            webSocket->setOnMessageCallback(
                [this, webSocket, rawConnection](const ix::WebSocketMessagePtr& message) {
                    if (message->type == ix::WebSocketMessageType::Open) {
                        publisher_.addConnection(webSocket);
                    } else if (message->type == ix::WebSocketMessageType::Message) {
                        try {
                            commandHandler_.handleMessage(message->str);
                        } catch (const std::exception& e) {
                            std::cerr << "Exception while handling message \"" << message->str << "\": " << e.what() << std::endl;
                        }
                    } else if (message->type == ix::WebSocketMessageType::Close) {
                        publisher_.removeConnection(rawConnection);
                    }
                });
        });
}

bool GameWebSocketServer::start() {
    auto result = server_.listen();
    if (!result.first) {
        std::cerr << "WebSocket server failed to listen: " << result.second << std::endl;
        return false;
    }
    server_.start();
    return true;
}

void GameWebSocketServer::wait() {
    server_.wait();
}

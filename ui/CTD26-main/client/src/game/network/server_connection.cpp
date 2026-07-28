#include "server_connection.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include "logging/logger.hpp"

ServerConnection::ServerConnection(const std::string& server_url) {
    ix::initNetSystem();
    socket_ = std::make_unique<ix::WebSocket>();
    socket_->setUrl(server_url);
    socket_->setOnMessageCallback([this](const ix::WebSocketMessagePtr& message) {
        if (message->type == ix::WebSocketMessageType::Open) {
            Logger::info("Connected to server");
            if (onOpen_) onOpen_();
        } else if (message->type == ix::WebSocketMessageType::Message) {
            if (onMessage_) onMessage_(message->str);
        } else if (message->type == ix::WebSocketMessageType::Error) {
            Logger::error("Connection error: " + message->errorInfo.reason);
        } else if (message->type == ix::WebSocketMessageType::Close) {
            Logger::info("Disconnected from server");
        }
    });}

void ServerConnection::start() {
    socket_->start();
}

ServerConnection::~ServerConnection() {
    socket_->stop();
}

void ServerConnection::send(const std::string& message) {
    socket_->send(message);
}

void ServerConnection::setOnMessage(std::function<void(const std::string&)> handler) {
    onMessage_ = std::move(handler);
}

void ServerConnection::setOnOpen(std::function<void()> handler) {
    onOpen_ = std::move(handler);
}
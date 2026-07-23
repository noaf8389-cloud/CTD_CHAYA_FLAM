#include "server_connection.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

ServerConnection::ServerConnection(const std::string& server_url) {
    ix::initNetSystem();
    socket_ = std::make_unique<ix::WebSocket>();
    socket_->setUrl(server_url);
}

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
    socket_->setOnMessageCallback([handler](const ix::WebSocketMessagePtr& message) {
        if (message->type == ix::WebSocketMessageType::Message) {
            handler(message->str);
        }
    });
}

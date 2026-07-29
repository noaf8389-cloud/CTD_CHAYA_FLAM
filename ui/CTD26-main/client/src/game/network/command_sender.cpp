#include "command_sender.hpp"

#include "server_connection.hpp"
#include "third_party/nlohmann/json.hpp"

using json = nlohmann::json;

CommandSender::CommandSender(ServerConnection& connection) : connection_(connection) {}

void CommandSender::send(const char* command, int row, int col) {
    json message{{"command", command}, {"row", row}, {"col", col}};
    connection_.send(message.dump());
}

void CommandSender::sendLogin(const std::string& username, const std::string& password) {
    json message{{"command", "login"}, {"username", username}, {"password", password}};
    connection_.send(message.dump());
}

void CommandSender::sendFindMatch() {
    connection_.send(json{{"command", "findMatch"}}.dump());
}

void CommandSender::sendCreateRoom() {
    connection_.send(json{{"command", "createRoom"}}.dump());
}

void CommandSender::sendJoinRoom(const std::string& roomCode) {
    connection_.send(json{{"command", "joinRoom"}, {"roomCode", roomCode}}.dump());
}

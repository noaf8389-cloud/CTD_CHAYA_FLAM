#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "bus/event_serialization.hpp"

class ServerConnection;
class EventBus;

// Local, client-only responses to the login/matchmaking commands. Unlike the
// other events here, the server sends these directly to one connection rather
// than broadcasting them through a match's event bus, so they aren't shared.
struct LoginResultReceived {
    bool success;
    int rating;
};
struct RoomCreatedReceived {
    std::string roomCode;
};
struct JoinRoomFailedReceived {};
struct NoMatchFoundReceived {};

class NetworkReceiver {
public:
    NetworkReceiver(ServerConnection& connection, EventBus& bus);

    // Parses a raw envelope message and publishes the matching event on the bus.
    void handleMessage(const std::string& raw);

private:
    EventBus& bus_;
    std::unordered_map<std::string, std::function<void(const json&)>> handlers_;
};

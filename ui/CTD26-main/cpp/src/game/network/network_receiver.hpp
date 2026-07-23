#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "../bus/event_serialization.hpp"

class ServerConnection;
class EventBus;

class NetworkReceiver {
public:
    NetworkReceiver(ServerConnection& connection, EventBus& bus);

    // Parses a raw envelope message and publishes the matching event on the bus.
    void handleMessage(const std::string& raw);

private:
    EventBus& bus_;
    std::unordered_map<std::string, std::function<void(const json&)>> handlers_;
};

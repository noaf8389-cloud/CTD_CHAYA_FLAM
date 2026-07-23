#include "network_publisher.hpp"

#include <algorithm>

#include "../bus/event_serialization.hpp"
#include "../game_session.hpp"

namespace {
    template <typename EventT>
    std::string envelope(const char* typeName, const EventT& event) {
        json j;
        j["type"] = typeName;
        j["payload"] = event;
        return j.dump();
    }
}

NetworkPublisher::NetworkPublisher(EventBus& bus, GameSession& gameSession) : bus_(bus), gameSession_(gameSession) {
    bus_.subscribe<MoveMadeEvent>([this](const MoveMadeEvent& e) {
        broadcast(envelope("MoveMadeEvent", e));
    });
    bus_.subscribe<PieceCapturedEvent>([this](const PieceCapturedEvent& e) {
        broadcast(envelope("PieceCapturedEvent", e));
    });
    bus_.subscribe<GameOverEvent>([this](const GameOverEvent& e) {
        broadcast(envelope("GameOverEvent", e));
    });
    bus_.subscribe<ScoreUpdatedEvent>([this](const ScoreUpdatedEvent& e) {
        broadcast(envelope("ScoreUpdatedEvent", e));
    });
    bus_.subscribe<MoveStartedEvent>([this](const MoveStartedEvent& e) {
        broadcast(envelope("MoveStartedEvent", e));
    });
    bus_.subscribe<JumpStartedEvent>([this](const JumpStartedEvent& e) {
        broadcast(envelope("JumpStartedEvent", e));
    });
    bus_.subscribe<JumpLandedEvent>([this](const JumpLandedEvent& e) {
        broadcast(envelope("JumpLandedEvent", e));
    });
    bus_.subscribe<RestEndedEvent>([this](const RestEndedEvent& e) {
        broadcast(envelope("RestEndedEvent", e));
    });
}

void NetworkPublisher::addConnection(std::shared_ptr<ix::WebSocket> connection) {
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        connections_.push_back(connection);
    }
    connection->send(envelope("GameStartedEvent", gameSession_.buildGameStartedEvent()));
}

void NetworkPublisher::removeConnection(ix::WebSocket* connection) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                        [connection](const std::shared_ptr<ix::WebSocket>& c) { return c.get() == connection; }),
        connections_.end());
}

void NetworkPublisher::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (auto& connection : connections_) {
        connection->send(message);
    }
}

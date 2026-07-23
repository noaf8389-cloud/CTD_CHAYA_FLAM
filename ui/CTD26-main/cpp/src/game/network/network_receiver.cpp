#include "network_receiver.hpp"

#include <iostream>

#include "server_connection.hpp"
#include "../bus/event_bus.hpp"

NetworkReceiver::NetworkReceiver(ServerConnection& connection, EventBus& bus) : bus_(bus) {
    handlers_["MoveMadeEvent"] = [this](const json& payload) { bus_.publish(payload.get<MoveMadeEvent>()); };
    handlers_["PieceCapturedEvent"] = [this](const json& payload) { bus_.publish(payload.get<PieceCapturedEvent>()); };
    handlers_["GameStartedEvent"] = [this](const json& payload) { bus_.publish(payload.get<GameStartedEvent>()); };
    handlers_["GameOverEvent"] = [this](const json& payload) { bus_.publish(payload.get<GameOverEvent>()); };
    handlers_["ScoreUpdatedEvent"] = [this](const json& payload) { bus_.publish(payload.get<ScoreUpdatedEvent>()); };
    handlers_["MoveStartedEvent"] = [this](const json& payload) { bus_.publish(payload.get<MoveStartedEvent>()); };
    handlers_["JumpStartedEvent"] = [this](const json& payload) { bus_.publish(payload.get<JumpStartedEvent>()); };
    handlers_["JumpLandedEvent"] = [this](const json& payload) { bus_.publish(payload.get<JumpLandedEvent>()); };
    handlers_["RestEndedEvent"] = [this](const json& payload) { bus_.publish(payload.get<RestEndedEvent>()); };

    connection.setOnMessage([this](const std::string& raw) { handleMessage(raw); });
}

void NetworkReceiver::handleMessage(const std::string& raw) {
    json parsed;
    try {
        parsed = json::parse(raw);
    } catch (const json::parse_error&) {
        return;
    }

    if (!parsed.contains("payload")) {
        return;
    }

    const auto it = handlers_.find(parsed.value("type", ""));
    if (it == handlers_.end()) {
        return;
    }

    try {
        it->second(parsed.at("payload"));
    } catch (const std::exception& e) {
        std::cerr << "Exception while handling event \"" << parsed.value("type", "") << "\": " << e.what() << std::endl;
    }
}

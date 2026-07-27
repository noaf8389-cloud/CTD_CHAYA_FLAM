#include "player_panel_tracker.hpp"
#include "bus/event_bus.hpp"
#include "bus/game_events.hpp"

PlayerPanelTracker::PlayerPanelTracker(EventBus& bus) {
    bus.subscribe<ScoreUpdatedEvent>([this](const ScoreUpdatedEvent& e) {
        if (e.color == 'w') white_.score = e.new_score;
        else black_.score = e.new_score;
    });

    bus.subscribe<MoveMadeEvent>([this](const MoveMadeEvent& e) {
        char color = e.piece_token.empty() ? 'w' : e.piece_token[0];
        std::string line = "(" + std::to_string(e.from.row) + "," + std::to_string(e.from.col) + ") -> ("
                          + std::to_string(e.to.row) + "," + std::to_string(e.to.col) + ")";
        if (color == 'w') white_.moves.push_back(line);
        else black_.moves.push_back(line);
    });
}

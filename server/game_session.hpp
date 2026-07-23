#pragma once

#include <mutex>

#include "bus/event_bus.hpp"
#include "bus/game_events.hpp"
#include "logic/model/game_state.hpp"
#include "logic/input/controller.hpp"

class GameSession {
public:
    GameSession(GameState& gameState, EventBus& bus)
        : gameState_(gameState), bus_(bus) {}

    void handleClick(int row, int col);
    void handleJump(int row, int col);

    /// Updates the game state and publishes events for completed actions.
    void update(long long deltaMs);

    /// Builds a snapshot of the current board as a GameStartedEvent, for a newly connected client.
    GameStartedEvent buildGameStartedEvent() const;

private:
    GameState& gameState_;
    EventBus& bus_;
    mutable std::recursive_mutex mutex_;
};

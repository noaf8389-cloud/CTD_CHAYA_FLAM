#pragma once

#include <mutex>

#include "model/board.hpp"
#include "bus/game_events.hpp"

class EventBus;

class ClientBoardState {
public:
    // Starts with an empty board and subscribes to game-started/move/capture/game-over events to sync with the server.
    explicit ClientBoardState(EventBus& bus);


    // Returns a snapshot of the board reflecting every event applied so far.
    Board board() const;

    // Returns true once a GameOverEvent has been received.
    bool is_game_over() const;

private:
    void onGameStarted(const GameStartedEvent& event);
    void onMoveMade(const MoveMadeEvent& event);
    void onPieceCaptured(const PieceCapturedEvent& event);
    void onGameOver(const GameOverEvent& event);

    mutable std::mutex mutex_;
    Board board_{0, 0};
    bool game_over_ = false;
};

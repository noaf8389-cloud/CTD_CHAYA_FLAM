// input/controller.hpp
#pragma once
#include "../model/game_state.hpp"
#include "../rules/piece_movement_rules.hpp"

class Controller {
public:
    // Handles a click at a pixel coordinate: selects a piece, or requests a move if one is already selected.
    // Returns the motion that was just queued, if a move was actually requested.
    static std::optional<Motion> handleClick(int row, int col, GameState& gameState);

    // Handles a jump at a pixel coordinate: starts an airborne jump for the piece at that cell.
    // Returns the position where the jump started, if one was actually started.
    static std::optional<Position> handleJump(int row, int col, GameState& gameState);

    private:
    static void handleClickWithNoSelection(const Position& clicked, GameState& gameState);
    static std::optional<Motion> handleClickWithSelection(const Position& clicked, const Position& selected, GameState& gameState);
};

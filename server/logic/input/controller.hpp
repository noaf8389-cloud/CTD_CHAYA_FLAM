// input/controller.hpp
#pragma once
#include "../model/game_state.hpp"
#include "../rules/piece_movement_rules.hpp"

class Controller {
public:
    // Handles a click on behalf of the given color: selects one of that color's pieces,
    // or requests a move if one of that color's pieces is already selected. Clicks on a
    // different color's piece are ignored.
    static std::optional<Motion> handleClick(int row, int col, GameState& gameState, char actingColor);

    // Handles a jump at a pixel coordinate: starts an airborne jump for the piece at that cell.
    // Returns the position where the jump started, if one was actually started and , if it belongs to that color.
    static std::optional<Position> handleJump(int row, int col, GameState& gameState, char actingColor);

private:
    static void handleClickWithNoSelection(const Position& clicked, GameState& gameState, char actingColor);
    static std::optional<Motion> handleClickWithSelection(const Position& clicked, const Position& selected, GameState& gameState, char actingColor);
};

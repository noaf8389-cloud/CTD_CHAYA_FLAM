// input/controller.hpp
#pragma once
#include "../model/game_state.hpp"
#include "../rules/rules_engine.hpp"

class Controller {
public:
    static void handleClick(int x, int y, GameState& gameState);

private:
    static void handleClickWithNoSelection(const Position& clicked, GameState& gameState);
    static void handleClickWithSelection(const Position& clicked, const Position& selected, GameState& gameState);
};

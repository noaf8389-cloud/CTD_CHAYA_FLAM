// https://github.com/noaf8389-cloud/CTD_CHAYA_FLAM.git

#include <iostream>
#include "io/board_parser.hpp"
#include "model/game_state.hpp"
#include "engine/game_engine.hpp"

int main() {
    Board board(0, 0);
    std::vector<std::string> commands;

    ParseError error = BoardParser::parse(std::cin, board, commands);

    if (error == ParseError::UNKNOWN_TOKEN) {
        std::cout << "ERROR UNKNOWN_TOKEN" << std::endl;
        return 0;
    }
    if (error == ParseError::ROW_WIDTH_MISMATCH) {
        std::cout << "ERROR ROW_WIDTH_MISMATCH" << std::endl;
        return 0;
    }

    GameState gameState(board);
    GameEngine::run(gameState, commands);

    return 0;
}
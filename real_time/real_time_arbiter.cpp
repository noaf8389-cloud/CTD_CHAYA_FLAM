#include "real_time_arbiter.hpp"

void RealTimeArbiter::applyCompletedMoves(GameState& gameState) {
    std::vector<Motion> completed = gameState.extractCompletedMoves();

    for (const Motion& motion : completed) {
        Board& board = gameState.getBoard();
        std::string token = board.getCell(motion.from.row, motion.from.col);
        if (token == Board::EMPTY_CELL) {
            continue;
        }

        std::string capturedToken = board.getCell(motion.to.row, motion.to.col);
        if (capturedToken.size() == 2 && capturedToken[1] == 'K') {
            gameState.endGame();
        }

        board.setCell(motion.from.row, motion.from.col, Board::EMPTY_CELL);
        board.setCell(motion.to.row, motion.to.col, token);
    }
}

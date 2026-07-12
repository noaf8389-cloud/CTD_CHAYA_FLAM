#include "real_time_arbiter.hpp"

void RealTimeArbiter::applyCompletedMoves(GameState& gameState) {
    std::vector<Motion> completed = gameState.extractCompletedMoves();

    for (const Motion& motion : completed) {
        Board& board = gameState.getBoard();
        std::string token = board.getCell(motion.from.row, motion.from.col);
        if (token == Board::EMPTY_CELL) {
            continue;
        }
        board.setCell(motion.from.row, motion.from.col, Board::EMPTY_CELL);
        board.setCell(motion.to.row, motion.to.col, token);
    }
}

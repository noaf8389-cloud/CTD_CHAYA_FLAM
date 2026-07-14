#include "real_time_arbiter.hpp"
#include "../rules/piece_movement_rules.hpp"
#include "../rules/piece_rules.hpp"

namespace {
    void promoteIfNeeded(const Position& position, Board& board) {
        std::string token = board.getCell(position.row, position.col);
        if (token.size() != 2 || token[1] != 'P') {
            return;
        }

        char color = token[0];
        int forwardStep = PieceRules::getForwardDirection(color);
        if (!PawnRule().isPromotionRow(forwardStep, position.row, board.getRowCount())) {
            return;
        }

        board.setCell(position.row, position.col, std::string(1, color) + "Q");
    }
}

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
        promoteIfNeeded(motion.to, board);
    }
}

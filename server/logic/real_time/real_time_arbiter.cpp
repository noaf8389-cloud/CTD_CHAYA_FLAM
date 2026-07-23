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

std::vector<CompletedMoveResult> RealTimeArbiter::applyCompletedMoves(GameState& gameState) {
    std::vector<Motion> completed = gameState.extractCompletedMoves();
    std::vector<CompletedMoveResult> results;

    for (const Motion& motion : completed) {
        Board& board = gameState.getBoard();
        std::string token = board.getCell(motion.from.row, motion.from.col);
        if (token == Board::EMPTY_CELL) {
            continue;
        }

        std::string destinationToken = board.getCell(motion.to.row, motion.to.col);
        CompletedMoveResult result;
        result.from = motion.from;
        result.to = motion.to;
        result.movedToken = token;

        if (gameState.hasActiveJumpAt(motion.to) && !PieceRules::isSameColor(token, destinationToken)) {
            result.captured = true;
            result.capturedToken = token;
            result.survivingToken = destinationToken;
            if (token.size() == 2 && token[1] == 'K') {
                gameState.endGame();
                result.gameEnded = true;
            }
            
            board.setCell(motion.from.row, motion.from.col, Board::EMPTY_CELL);
            gameState.clearJumpAt(motion.to);
            results.push_back(result);
            continue;
        }
        
        if (destinationToken.size() == 2 && destinationToken[1] == 'K') {
            gameState.endGame();
            result.gameEnded = true;
        }

        if (destinationToken != Board::EMPTY_CELL) {
            result.captured = true;
            result.capturedToken = destinationToken;
            result.survivingToken = token;
        }

        board.setCell(motion.from.row, motion.from.col, Board::EMPTY_CELL);
        board.setCell(motion.to.row, motion.to.col, token);
        promoteIfNeeded(motion.to, board);

        std::string finalToken = board.getCell(motion.to.row, motion.to.col);
        result.movedToken = finalToken;
        if (result.captured) {
            result.survivingToken = finalToken;
        }

        gameState.startLongRest(motion.to);
        results.push_back(result);
    }
    return results;
}

std::vector<Position> RealTimeArbiter::applyExpiredJumps(GameState& gameState) {
    std::vector<Position> expired = gameState.extractExpiredJumps();
    for (const Position& position : expired) {
        gameState.startShortRest(position);
    }
    return expired;
}

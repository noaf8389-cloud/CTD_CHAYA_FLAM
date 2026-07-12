#include "queen_rule.hpp"
#include "movement_helper.hpp"

bool QueenRule::isLegalMove(const Position& from, const Position& to, const Board& board) {
    bool validShape = MovementHelper::isStraightLine(from, to) || MovementHelper::isDiagonal(from, to);
    if (!validShape) {
        return false;
    }
    return MovementHelper::isPathClear(from, to, board);
}

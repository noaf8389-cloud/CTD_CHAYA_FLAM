#include "rook_rule.hpp"
#include "movement_helper.hpp"

bool RookRule::isLegalMove(const Position& from, const Position& to, const Board& board) {
    if (!MovementHelper::isStraightLine(from, to)) {
        return false;
    }
    return MovementHelper::isPathClear(from, to, board);
}

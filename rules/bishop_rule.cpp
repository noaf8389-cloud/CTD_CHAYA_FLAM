#include "bishop_rule.hpp"
#include "movement_helper.hpp"

bool BishopRule::isLegalMove(const Position& from, const Position& to, const Board& board) {
    if (!MovementHelper::isDiagonal(from, to)) {
        return false;
    }
    return MovementHelper::isPathClear(from, to, board);
}

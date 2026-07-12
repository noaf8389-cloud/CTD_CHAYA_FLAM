#include "king_rule.hpp"
#include <cstdlib>

bool KingRule::isLegalMove(const Position& from, const Position& to, const Board& board) {
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);

    if (rowDiff == 0 && colDiff == 0) {
        return false;
    }

    return rowDiff <= 1 && colDiff <= 1;
}

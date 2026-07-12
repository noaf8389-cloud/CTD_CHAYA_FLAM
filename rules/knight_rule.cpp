#include "knight_rule.hpp"
#include <cstdlib>

bool KnightRule::isLegalMove(const Position& from, const Position& to, const Board& board) {
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);

    return (rowDiff == 1 && colDiff == 2) || (rowDiff == 2 && colDiff == 1);
}

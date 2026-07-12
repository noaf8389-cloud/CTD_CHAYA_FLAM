#include "movement_helper.hpp"
#include <cstdlib>

namespace {
    int step(int from, int to) {
        if (to > from) return 1;
        if (to < from) return -1;
        return 0;
    }
}

bool MovementHelper::isStraightLine(const Position& from, const Position& to) {
    bool sameRow = from.row == to.row;
    bool sameCol = from.col == to.col;
    return sameRow != sameCol;
}

bool MovementHelper::isDiagonal(const Position& from, const Position& to) {
    int rowDiff = std::abs(from.row - to.row);
    int colDiff = std::abs(from.col - to.col);
    return rowDiff == colDiff && rowDiff != 0;
}

bool MovementHelper::isPathClear(const Position& from, const Position& to, const Board& board) {
    int rowStep = step(from.row, to.row);
    int colStep = step(from.col, to.col);

    int row = from.row + rowStep;
    int col = from.col + colStep;

    while (row != to.row || col != to.col) {
        if (board.getCell(row, col) != Board::EMPTY_CELL) {
            return false;
        }
        row += rowStep;
        col += colStep;
    }

    return true;
}

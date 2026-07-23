#include "board_mapper.hpp"

bool BoardMapper::isOutOfBounds(int row, int col, int rowCount, int colCount) {
    return row < 0 || col < 0 || row >= rowCount || col >= colCount;
}

std::optional<Position> BoardMapper::toPosition(int row, int col, int rowCount, int colCount) {
    if (isOutOfBounds(row, col, rowCount, colCount)) {
        return std::nullopt;
    }
    return Position{row, col};
}

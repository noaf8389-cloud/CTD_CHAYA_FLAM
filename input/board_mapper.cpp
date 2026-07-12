#include "board_mapper.hpp"

bool BoardMapper::isOutOfBounds(int x, int y, int rowCount, int colCount) {
    if (x < 0 || y < 0) {
        return true;
    }

    int col = x / CELL_SIZE_PIXELS;
    int row = y / CELL_SIZE_PIXELS;

    return row >= rowCount || col >= colCount;
}

std::optional<Position> BoardMapper::toPosition(int x, int y, int rowCount, int colCount) {
    if (isOutOfBounds(x, y, rowCount, colCount)) {
        return std::nullopt;
    }

    int col = x / CELL_SIZE_PIXELS;
    int row = y / CELL_SIZE_PIXELS;

    return Position{row, col};
}

#pragma once
#include <optional>
#include "../model/Position.hpp"

class BoardMapper {
public:
    // Checks whether a row/col falls outside the board's grid.
    static bool isOutOfBounds(int row, int col, int rowCount, int colCount);
    // Wraps a row/col into a board Position, or nullopt if it's out of bounds.
    static std::optional<Position> toPosition(int row, int col, int rowCount, int colCount);
};

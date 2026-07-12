#pragma once
#include <optional>
#include "../model/Position.hpp"

class BoardMapper {
public:
    static const int CELL_SIZE_PIXELS = 100;

    static bool isOutOfBounds(int x, int y, int rowCount, int colCount);
    static std::optional<Position> toPosition(int x, int y, int rowCount, int colCount);
};

#pragma once
#include "../model/Position.hpp"
#include "../model/board.hpp"

class MovementHelper {
public:
    static bool isStraightLine(const Position& from, const Position& to);
    static bool isDiagonal(const Position& from, const Position& to);
    static bool isPathClear(const Position& from, const Position& to, const Board& board);
};

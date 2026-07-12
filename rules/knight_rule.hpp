#pragma once
#include "../model/Position.hpp"
#include "../model/board.hpp"

class KnightRule {
public:
    static bool isLegalMove(const Position& from, const Position& to, const Board& board);
};

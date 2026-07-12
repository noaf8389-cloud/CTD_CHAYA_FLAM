#pragma once
#include "../model/Position.hpp"
#include "../model/board.hpp"

class KingRule {
public:
    static bool isLegalMove(const Position& from, const Position& to, const Board& board);
};

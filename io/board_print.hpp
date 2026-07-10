#pragma once
#include <ostream>
#include "../model/board.hpp"

class BoardPrinter {
public:
    static void print(const Board& board, std::ostream& out);
};
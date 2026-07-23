#pragma once
#include <ostream>
#include "../model/board.hpp"

class BoardPrinter {
public:
    // Writes the board's current cell tokens, row by row, to the given output stream.
    static void print(const Board& board, std::ostream& out);
};
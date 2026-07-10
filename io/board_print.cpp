#include "board_print.hpp"

void BoardPrinter::print(const Board& board, std::ostream& out) {
    for (int row = 0; row < board.getRowCount(); ++row) {
        for (int col = 0; col < board.getColCount(); ++col) {
            if (col > 0) {
                out << " ";
            }
            out << board.getCell(row, col);
        }
        out << "\n";
    }
}
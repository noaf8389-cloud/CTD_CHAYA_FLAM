#include "../catch2/catch_amalgamated.hpp"
#include <sstream>
#include "../../logic/io/board_print.hpp"

TEST_CASE("print prints each row space-separated with newline") {
    Board board(2, 2);
    board.setCell(0, 0, "wK");
    board.setCell(1, 1, "bQ");

    std::ostringstream output;
    BoardPrinter::print(board, output);

    REQUIRE(output.str() == "wK .\n. bQ\n");
}

TEST_CASE("printing an empty 0x0 board prints nothing") {
    Board board(0, 0);
    std::ostringstream output;
    BoardPrinter::print(board, output);
    REQUIRE(output.str() == "");
}

TEST_CASE("printing a single cell board") {
    Board board(1, 1);
    board.setCell(0, 0, "wK");
    std::ostringstream output;
    BoardPrinter::print(board, output);
    REQUIRE(output.str() == "wK\n");
}

TEST_CASE("printing a single row board") {
    Board board(1, 3);
    board.setCell(0, 1, "bQ");
    std::ostringstream output;
    BoardPrinter::print(board, output);
    REQUIRE(output.str() == ". bQ .\n");
}

TEST_CASE("printing a single column board") {
    Board board(3, 1);
    board.setCell(1, 0, "wR");
    std::ostringstream output;
    BoardPrinter::print(board, output);
    REQUIRE(output.str() == ".\nwR\n.\n");
}

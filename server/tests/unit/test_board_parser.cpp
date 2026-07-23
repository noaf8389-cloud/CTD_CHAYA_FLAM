#include "../catch2/catch_amalgamated.hpp"
#include <sstream>
#include "../../logic/io/board_parser.hpp"

TEST_CASE("parse_empty_board_3x3") {
    std::istringstream input(
        "Board:\n. . .\n. . .\n. . .\nCommands:\nprint board\n");
    Board board(0, 0);
    std::vector<std::string> commands;

    ParseError error = BoardParser::parse(input, board, commands);

    REQUIRE(error == ParseError::NONE);
    REQUIRE(board.getRowCount() == 3);
    REQUIRE(board.getColCount() == 3);
}

TEST_CASE("parse_rectangular_board_3x4") {
    std::istringstream input(
        "Board:\nwK . . bK\n. . . .\nwR . . bR\nCommands:\nprint board\n");
    Board board(0, 0);
    std::vector<std::string> commands;

    ParseError error = BoardParser::parse(input, board, commands);

    REQUIRE(error == ParseError::NONE);
    REQUIRE(board.getRowCount() == 3);
    REQUIRE(board.getColCount() == 4);
    REQUIRE(board.getCell(0, 0) == "wK");
    REQUIRE(board.getCell(0, 3) == "bK");
}

TEST_CASE("parse_piece_tokens_and_colors") {
    std::istringstream input(
        "Board:\nwK . bQ\n. wN .\nbP . wR\nCommands:\nprint board\n");
    Board board(0, 0);
    std::vector<std::string> commands;

    ParseError error = BoardParser::parse(input, board, commands);

    REQUIRE(error == ParseError::NONE);
    REQUIRE(board.getCell(1, 1) == "wN");
}

TEST_CASE("reject_unknown_token") {
    std::istringstream input("Board:\nwK xZ\n. .\nCommands:\n");
    Board board(0, 0);
    std::vector<std::string> commands;

    ParseError error = BoardParser::parse(input, board, commands);

    REQUIRE(error == ParseError::UNKNOWN_TOKEN);
}

TEST_CASE("reject_row_width_mismatch") {
    std::istringstream input("Board:\nwK . .\n. bK\nCommands:\n");
    Board board(0, 0);
    std::vector<std::string> commands;

    ParseError error = BoardParser::parse(input, board, commands);

    REQUIRE(error == ParseError::ROW_WIDTH_MISMATCH);
}

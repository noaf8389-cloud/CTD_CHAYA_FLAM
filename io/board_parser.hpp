#pragma once
#include <istream>
#include <vector>
#include <string>
#include "../model/board.hpp"

enum class ParseError {
    NONE,
    UNKNOWN_TOKEN,
    ROW_WIDTH_MISMATCH
};

class BoardParser {
public:
    static ParseError parse(std::istream& input, Board& board, std::vector<std::string>& commands);
private:
    static std::vector<std::vector<std::string>> readBoardSection(std::istream& input, std::vector<std::string>& commands);
    static ParseError validate(const std::vector<std::vector<std::string>>& rows);
    static Board buildBoard(const std::vector<std::vector<std::string>>& rows);
};
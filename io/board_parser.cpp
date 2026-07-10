#include "board_parser.hpp"
#include "../rules/piece_rules.hpp"
#include <sstream>

std::vector<std::vector<std::string>> BoardParser::readBoardSection(std::istream& input, std::vector<std::string>& commands) {
    std::vector<std::vector<std::string>> rawRows;
    std::string line;
    bool inCommandsSection = false;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line == "Board:") {
            continue;
        }
        if (line == "Commands:") {
            inCommandsSection = true;
            continue;
        }
        if (inCommandsSection) {
            if (!line.empty()) {
                commands.push_back(line);
            }
            continue;
        }

        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        if (!tokens.empty()) {
            rawRows.push_back(tokens);
        }
    }

    return rawRows;
}

ParseError BoardParser::validate(const std::vector<std::vector<std::string>>& rows) {
    for (const std::vector<std::string>& row : rows) {
        for (const std::string& token : row) {
            if (!PieceRules::isValidToken(token)) {
                return ParseError::UNKNOWN_TOKEN;
            }
        }
    }

    if (!rows.empty()) {
        size_t width = rows[0].size();
        for (const std::vector<std::string>& row : rows) {
            if (row.size() != width) {
                return ParseError::ROW_WIDTH_MISMATCH;
            }
        }
    }

    return ParseError::NONE;
}

Board BoardParser::buildBoard(const std::vector<std::vector<std::string>>& rows) {
    int rowCount = static_cast<int>(rows.size());
    int colCount = rowCount > 0 ? static_cast<int>(rows[0].size()) : 0;
    Board board(rowCount, colCount);

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            board.setCell(row, col, rows[row][col]);
        }
    }

    return board;
}

ParseError BoardParser::parse(std::istream& input, Board& board, std::vector<std::string>& commands) {
    std::vector<std::vector<std::string>> rawRows = readBoardSection(input, commands);

    ParseError error = validate(rawRows);
    if (error != ParseError::NONE) {
        return error;
    }

    board = buildBoard(rawRows);
    return ParseError::NONE;
}
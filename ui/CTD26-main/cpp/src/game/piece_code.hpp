#pragma once

#include <cctype>
#include <string>

// Converts a logic token ("wP": color+type) into the asset code used by PieceAssets ("PW": type+color).
inline std::string token_to_piece_code(const std::string& token) {
    return std::string(1, token[1]) + std::string(1, static_cast<char>(std::toupper(token[0])));
}

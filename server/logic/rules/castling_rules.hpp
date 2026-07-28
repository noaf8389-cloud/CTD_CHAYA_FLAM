#pragma once

#include <functional>
#include <optional>
#include "model/Position.hpp"
#include "model/board.hpp"

struct CastlingMove {
    Position kingFrom;
    Position kingTo;
    Position rookFrom;
    Position rookTo;
};

// Detects and validates castling.
// Takes the history as a predicate so this class
class CastlingRules {
public:
    static std::optional<CastlingMove> tryCastle(const Position& from, const Position& to, const Board& board,
                                                  const std::function<bool(const Position&)>& wasVacated);
};

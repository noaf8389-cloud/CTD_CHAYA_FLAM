#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

#include "game_match.hpp"
#include "db/player_account_store.hpp"
#include "logic/model/game_state.hpp"

// Owns every currently-active GameMatch on the server. Creates matches, finds
// which match a connection belongs to, and reaps matches once they end.
class GameRegistry {
public:
    using GameId = std::size_t;

    // Creates a new match on startingBoard for exactly these two connections,
    // starts tracking it (including auto-removal once it ends), and returns its id.
    // accounts must outlive the match.
    GameId createMatch(Board, PlayerAccountStore&, const std::string& player1Username, const std::string& player2Username);
    // The match this connection currently belongs to, or nullptr if it isn't in
    // one (never matched, or its match already ended and was pruned).
    GameMatch* matchFor(const std::string& username);

    // Advances every tracked match by deltaMs. Call once per server tick.
    void updateAll(long long deltaMs);

    // Number of matches currently tracked. Mainly for tests/diagnostics.
    std::size_t matchCount() const;

private:
    // Recursive: updateAll() holds this lock while calling match->update(), and a
    // match ending fires GameOverEvent synchronously on the same thread, which the
    // subscription below handles by re-entering this same lock.
    mutable std::recursive_mutex mutex_;
    std::unordered_map<GameId, std::unique_ptr<GameMatch>> matches_;
    std::unordered_map<std::string, GameId> usernameToGame_;
    std::vector<GameId> finishedGameIds_;
    GameId nextId_ = 1;
};

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
    using RoomCode = std::string;

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

    // Creates a new room with only its creator known; returns a short shareable
    // code a second player/spectators join by. accounts must outlive the match.
    RoomCode createRoom(Board startingBoard, PlayerAccountStore& accounts, const std::string& creatorUsername);

    // Adds a player or spectator to an existing room by its code (color
    // assignment works exactly like any match: the third+ joiner gets no color,
    // i.e. a read-only spectator). Returns nullptr if the code is unknown.
    GameMatch* joinRoom(const RoomCode& roomCode, const std::string& joinerUsername);

private:
    GameId insertNewMatch(Board startingBoard, PlayerAccountStore& accounts);

    mutable std::recursive_mutex mutex_;
    std::unordered_map<GameId, std::unique_ptr<GameMatch>> matches_;
    std::unordered_map<std::string, GameId> usernameToGame_;
    std::unordered_map<RoomCode, GameId> roomCodeToGame_;
    std::vector<GameId> finishedGameIds_;
    GameId nextId_ = 1;
};

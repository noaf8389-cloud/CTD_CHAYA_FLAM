#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "matches/game_registry.hpp"
#include "db/player_account_store.hpp"
#include "logic/model/game_state.hpp"

// Holds a queue of players waiting for a match and pairs them up by rating proximity.
class Matchmaker {
public:
    using NoMatchFoundHandler = std::function<void(const std::string& username)>;

    static constexpr long long kDefaultTimeoutMs = 60000;

    // Builds a Matchmaker bound to a shared GameRegistry, account store, and the
    // board template used for every match it creates.
    Matchmaker(GameRegistry& gameRegistry, PlayerAccountStore& accounts, Board startingBoard,
               long long timeoutMs = kDefaultTimeoutMs);

    // Adds a player to the waiting queue by rating; ignored if already waiting.
    void enqueue(const std::string& username, int rating);

    // Removes a waiting player from the queue; safe to call if they aren't in it.
    void dequeue(const std::string& username);

    // Pairs up waiting players by rating and evicts anyone past the timeout.
    void tick();

    // Registers the callback fired for each player evicted by a timeout.
    void setOnNoMatchFound(NoMatchFoundHandler handler);

    // Returns the number of players currently waiting.
    std::size_t waitingCount() const;

private:
    struct WaitingPlayer {
        std::string username;
        int rating;
        std::chrono::steady_clock::time_point joinedAt;
    };

    static constexpr int kRatingRange = 100;

    mutable std::mutex mutex_;
    GameRegistry& gameRegistry_;
    PlayerAccountStore& accounts_;
    Board startingBoard_;
    long long timeoutMs_;
    std::vector<WaitingPlayer> waiting_;
    NoMatchFoundHandler onNoMatchFound_;
};

#include "matchmaker.hpp"

#include <algorithm>
#include <cmath>

Matchmaker::Matchmaker(GameRegistry& gameRegistry, PlayerAccountStore& accounts, Board startingBoard,
                        long long timeoutMs)
    : gameRegistry_(gameRegistry),
      accounts_(accounts),
      startingBoard_(std::move(startingBoard)),
      timeoutMs_(timeoutMs) {}

void Matchmaker::enqueue(const std::string& username, int rating) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& player : waiting_) {
        if (player.username == username) return;
    }
    waiting_.push_back(WaitingPlayer{username, rating, std::chrono::steady_clock::now()});
}

void Matchmaker::dequeue(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    waiting_.erase(
        std::remove_if(waiting_.begin(), waiting_.end(),
                        [&username](const WaitingPlayer& p) { return p.username == username; }),
        waiting_.end());
}

void Matchmaker::tick() {
    std::vector<std::string> timedOut;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        bool foundPair = true;
        while (foundPair) {
            foundPair = false;
            for (std::size_t i = 0; i < waiting_.size() && !foundPair; ++i) {
                for (std::size_t j = i + 1; j < waiting_.size(); ++j) {
                    if (std::abs(waiting_[i].rating - waiting_[j].rating) <= kRatingRange) {
                        std::string player1 = waiting_[i].username;
                        std::string player2 = waiting_[j].username;

                        waiting_.erase(waiting_.begin() + j);
                        waiting_.erase(waiting_.begin() + i);

                        gameRegistry_.createMatch(startingBoard_, accounts_, player1, player2);

                        foundPair = true;
                        break;
                    }
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        for (auto it = waiting_.begin(); it != waiting_.end(); ) {
            auto waitedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->joinedAt).count();
            if (waitedMs >= timeoutMs_) {
                timedOut.push_back(it->username);
                it = waiting_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Fired outside the lock: onNoMatchFound_ is caller-supplied code (e.g. it may
    // need to lock GameWebSocketServer's own connection state), and that caller also
    // calls back into enqueue()/dequeue() while holding its own lock. Calling the
    // handler while still holding mutex_ here would risk a lock-order inversion
    // between the two mutexes.
    if (onNoMatchFound_) {
        for (const std::string& username : timedOut) {
            onNoMatchFound_(username);
        }
    }
}

void Matchmaker::setOnNoMatchFound(NoMatchFoundHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    onNoMatchFound_ = std::move(handler);
}

std::size_t Matchmaker::waitingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return waiting_.size();
}

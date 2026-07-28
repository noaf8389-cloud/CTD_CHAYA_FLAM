#include "game_registry.hpp"

#include "bus/game_events.hpp"

GameRegistry::GameId GameRegistry::createMatch(Board startingBoard, PlayerAccountStore& accounts,
                                                const std::string& player1Username, const std::string& player2Username) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    GameId id = nextId_++;
    auto match = std::make_unique<GameMatch>(std::move(startingBoard), accounts);

    match->bus().subscribe<GameOverEvent>([this, id](const GameOverEvent&) {
        std::lock_guard<std::recursive_mutex> innerLock(mutex_);
        finishedGameIds_.push_back(id);
    });

    usernameToGame_[player1Username] = id;
    usernameToGame_[player2Username] = id;
    matches_[id] = std::move(match);
    return id;
}

GameMatch* GameRegistry::matchFor(const std::string& username) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = usernameToGame_.find(username);
    if (it == usernameToGame_.end()) return nullptr;

    auto matchIt = matches_.find(it->second);
    return matchIt != matches_.end() ? matchIt->second.get() : nullptr;
}

void GameRegistry::updateAll(long long deltaMs) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto& [id, match] : matches_) {
        match->update(deltaMs);
    }

    for (GameId id : finishedGameIds_) {
        matches_.erase(id);
    }
    finishedGameIds_.clear();

    for (auto it = usernameToGame_.begin(); it != usernameToGame_.end(); ) {
        if (matches_.find(it->second) == matches_.end()) {
            it = usernameToGame_.erase(it);
        } else {
            ++it;
        }
    }
}

std::size_t GameRegistry::matchCount() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return matches_.size();
}

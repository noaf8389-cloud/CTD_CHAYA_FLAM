#include "game_registry.hpp"

#include <random>

#include "bus/game_events.hpp"

namespace {
    std::string generateRoomCode() {
        static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> pick(0, sizeof(alphabet) - 2);

        std::string code(6, ' ');
        for (char& c : code) c = alphabet[pick(rng)];
        return code;
    }
}

GameRegistry::GameId GameRegistry::insertNewMatch(Board startingBoard, PlayerAccountStore& accounts) {
    GameId id = nextId_++;
    auto match = std::make_unique<GameMatch>(std::move(startingBoard), accounts);

    match->bus().subscribe<GameOverEvent>([this, id](const GameOverEvent&) {
        std::lock_guard<std::recursive_mutex> innerLock(mutex_);
        finishedGameIds_.push_back(id);
    });

    matches_[id] = std::move(match);
    return id;
}


GameRegistry::GameId GameRegistry::createMatch(Board startingBoard, PlayerAccountStore& accounts,
                                                const std::string& player1Username, const std::string& player2Username) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    GameId id = insertNewMatch(std::move(startingBoard), accounts);
    usernameToGame_[player1Username] = id;
    usernameToGame_[player2Username] = id;
    return id;
}

GameRegistry::RoomCode GameRegistry::createRoom(Board startingBoard, PlayerAccountStore& accounts, const std::string& creatorUsername) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    GameId id = insertNewMatch(std::move(startingBoard), accounts);
    usernameToGame_[creatorUsername] = id;

    RoomCode code;
    do {
        code = generateRoomCode();
    } while (roomCodeToGame_.find(code) != roomCodeToGame_.end());
    roomCodeToGame_[code] = id;
    return code;
}

GameMatch* GameRegistry::joinRoom(const RoomCode& roomCode, const std::string& joinerUsername) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto codeIt = roomCodeToGame_.find(roomCode);
    if (codeIt == roomCodeToGame_.end()) return nullptr;

    auto matchIt = matches_.find(codeIt->second);
    if (matchIt == matches_.end()) return nullptr;

    usernameToGame_[joinerUsername] = codeIt->second;
    return matchIt->second.get();
}

GameMatch* GameRegistry::matchFor(const std::string& username) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = usernameToGame_.find(username);
    if (it == usernameToGame_.end()) return nullptr;

    auto matchIt = matches_.find(it->second);
    return matchIt != matches_.end() ? matchIt->second.get() : nullptr;
}

void GameRegistry::updateAll(long long deltaMs) {
    std::vector<GameMatch*> active;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        active.reserve(matches_.size());
        for (auto& [id, match] : matches_) {
            active.push_back(match.get());
        }
    }

    // Runs outside mutex_: each match's update() broadcasts over the network to
    // its own connections, which shouldn't block unrelated registry lookups
    // (matchFor/createMatch/createRoom/joinRoom) for the whole server tick.
    for (GameMatch* match : active) {
        match->update(deltaMs);
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (GameId id : finishedGameIds_) {
        matches_.erase(id);
    }
    finishedGameIds_.clear();

    for (auto it = usernameToGame_.begin(); it != usernameToGame_.end(); ) {
        if (matches_.find(it->second) == matches_.end()) it = usernameToGame_.erase(it);
        else ++it;
    }

    for (auto it = roomCodeToGame_.begin(); it != roomCodeToGame_.end(); ) {
        if (matches_.find(it->second) == matches_.end()) it = roomCodeToGame_.erase(it);
        else ++it;
    }
}

std::size_t GameRegistry::matchCount() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return matches_.size();
}

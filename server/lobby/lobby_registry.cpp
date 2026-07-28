#include "lobby_registry.hpp"

LobbyRegistry::ConnectionId LobbyRegistry::registerConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    ConnectionId id = nextId_++;
    connections_.insert(id);
    return id;
}

void LobbyRegistry::unregisterConnection(ConnectionId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.erase(id);
    identities_.erase(id);
}

void LobbyRegistry::identify(ConnectionId id, const std::string& username, int rating) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connections_.find(id) == connections_.end()) return;

    identities_[id] = Identity{username, rating};
    if (onIdentified_) onIdentified_(id, username, rating);
}

std::optional<std::string> LobbyRegistry::usernameFor(ConnectionId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = identities_.find(id);
    if (it == identities_.end()) return std::nullopt;
    return it->second.username;
}

std::optional<int> LobbyRegistry::ratingFor(ConnectionId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = identities_.find(id);
    if (it == identities_.end()) return std::nullopt;
    return it->second.rating;
}

bool LobbyRegistry::isIdentified(ConnectionId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return identities_.find(id) != identities_.end();
}

void LobbyRegistry::setOnIdentified(IdentifiedHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    onIdentified_ = std::move(handler);
}

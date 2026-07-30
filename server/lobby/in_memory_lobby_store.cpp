#include "in_memory_lobby_store.hpp"

LobbyStore::ConnectionId InMemoryLobbyStore::nextConnectionId() {
    std::lock_guard<std::mutex> lock(mutex_);
    return nextId_++;
}

void InMemoryLobbyStore::addConnection(ConnectionId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.insert(id);
}

void InMemoryLobbyStore::removeConnection(ConnectionId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.erase(id);
    identities_.erase(id);
}

bool InMemoryLobbyStore::hasConnection(ConnectionId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.find(id) != connections_.end();
}

void InMemoryLobbyStore::setIdentity(ConnectionId id, const std::string& username, int rating) {
    std::lock_guard<std::mutex> lock(mutex_);
    identities_[id] = Identity{username, rating};
}

std::optional<LobbyStore::Identity> InMemoryLobbyStore::identityFor(ConnectionId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = identities_.find(id);
    if (it == identities_.end()) return std::nullopt;
    return it->second;
}

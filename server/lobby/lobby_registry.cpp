#include "lobby_registry.hpp"

#include "in_memory_lobby_store.hpp"

LobbyRegistry::LobbyRegistry()
    : ownedStore_(std::make_unique<InMemoryLobbyStore>()), store_(*ownedStore_) {}

LobbyRegistry::LobbyRegistry(LobbyStore& store) : store_(store) {}

LobbyRegistry::ConnectionId LobbyRegistry::registerConnection() {
    ConnectionId id = store_.nextConnectionId();
    store_.addConnection(id);
    return id;
}

void LobbyRegistry::unregisterConnection(ConnectionId id) {
    store_.removeConnection(id);
}

void LobbyRegistry::identify(ConnectionId id, const std::string& username, int rating) {
    if (!store_.hasConnection(id)) return;
    store_.setIdentity(id, username, rating);

    IdentifiedHandler handler;
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler = onIdentified_;
    }
    if (handler) handler(id, username, rating);
}

std::optional<std::string> LobbyRegistry::usernameFor(ConnectionId id) const {
    auto identity = store_.identityFor(id);
    return identity ? std::optional(identity->username) : std::nullopt;
}

std::optional<int> LobbyRegistry::ratingFor(ConnectionId id) const {
    auto identity = store_.identityFor(id);
    return identity ? std::optional(identity->rating) : std::nullopt;
}

bool LobbyRegistry::isIdentified(ConnectionId id) const {
    return store_.identityFor(id).has_value();
}

void LobbyRegistry::setOnIdentified(IdentifiedHandler handler) {
    std::lock_guard<std::mutex> lock(handlerMutex_);
    onIdentified_ = std::move(handler);
}

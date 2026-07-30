#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "lobby_store.hpp"

class LobbyRegistry {
public:
    using ConnectionId = LobbyStore::ConnectionId;
    using IdentifiedHandler = std::function<void(ConnectionId id, const std::string& username, int rating)>;

    // Default: owns an in-process InMemoryLobbyStore (today's behavior, unchanged).
    LobbyRegistry();
    // Injected: delegates to an externally-owned store (e.g. a future RedisLobbyStore),
    // shared across however many processes point at it.
    explicit LobbyRegistry(LobbyStore& store);

    // Registers a brand-new, not-yet-identified WebSocket connection; returns its id.
    ConnectionId registerConnection();

    // Removes a connection entirely (WS closed). Safe even if never identified.
    void unregisterConnection(ConnectionId id);

    // Records the username/rating for a registered connection (call after a successful
    // PlayerAccountStore login/register). Does nothing for an unknown id.
    void identify(ConnectionId id, const std::string& username, int rating);

    // The username for a connection, if identify() was called for it.
    std::optional<std::string> usernameFor(ConnectionId id) const;

    // The rating for a connection, if identify() was called for it.
    std::optional<int> ratingFor(ConnectionId id) const;

    // True once identify() has recorded an identity for this connection.
    bool isIdentified(ConnectionId id) const;

    // Fires whenever identify() successfully records an identity — lets a listener
    // (e.g. a future Matchmaker) react without LobbyRegistry knowing who's listening.
    void setOnIdentified(IdentifiedHandler handler);

private:
    std::unique_ptr<LobbyStore> ownedStore_;
    LobbyStore& store_;

    std::mutex handlerMutex_;
    IdentifiedHandler onIdentified_;
};

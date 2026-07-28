#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

class LobbyRegistry {
public:
    using ConnectionId = std::size_t;
    using IdentifiedHandler = std::function<void(ConnectionId id, const std::string& username, int rating)>;

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
    struct Identity { std::string username; int rating; };

    mutable std::mutex mutex_;
    std::unordered_set<ConnectionId> connections_;
    std::unordered_map<ConnectionId, Identity> identities_;
    ConnectionId nextId_ = 1;
    IdentifiedHandler onIdentified_;
};

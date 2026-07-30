#pragma once

#include <cstddef>
#include <optional>
#include <string>

// Storage for LobbyRegistry's connection/identity state — the part of the lobby
// that needs to be shared once more than one server process exists (Phase 5+).
// LobbyRegistry keeps its own orchestration logic (the onIdentified callback);
// this interface owns only the data, exactly like PlayerAccountStore owns only
// account data for AuthService.
class LobbyStore {
public:
    using ConnectionId = std::size_t;
    struct Identity { std::string username; int rating; };

    virtual ~LobbyStore() = default;

    // Atomically allocates a new connection id — must be safe to call concurrently
    // across however many processes share this store.
    virtual ConnectionId nextConnectionId() = 0;

    virtual void addConnection(ConnectionId id) = 0;
    virtual void removeConnection(ConnectionId id) = 0;
    virtual bool hasConnection(ConnectionId id) const = 0;

    virtual void setIdentity(ConnectionId id, const std::string& username, int rating) = 0;
    virtual std::optional<Identity> identityFor(ConnectionId id) const = 0;
};

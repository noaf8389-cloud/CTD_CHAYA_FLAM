#pragma once

#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "lobby_store.hpp"

// Today's LobbyRegistry behavior, unchanged — everything lives in this one process.
class InMemoryLobbyStore : public LobbyStore {
public:
    ConnectionId nextConnectionId() override;
    void addConnection(ConnectionId id) override;
    void removeConnection(ConnectionId id) override;
    bool hasConnection(ConnectionId id) const override;
    void setIdentity(ConnectionId id, const std::string& username, int rating) override;
    std::optional<Identity> identityFor(ConnectionId id) const override;

private:
    mutable std::mutex mutex_;
    std::unordered_set<ConnectionId> connections_;
    std::unordered_map<ConnectionId, Identity> identities_;
    ConnectionId nextId_ = 1;
};

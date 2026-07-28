#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using PlayerId = std::size_t;
using DisconnectHandler = std::function<void(char color, long long graceDurationMs)>;
using ReconnectHandler = std::function<void(char color)>;

// Assigns each connecting client one of a fixed set of colors, in order.
// Connections beyond the available colors remain unassigned (e.g. spectators).
class PlayerRegistry {
public:
    explicit PlayerRegistry(std::vector<char> colors = {'w', 'b'});

    // Assigns the next available color to a new connection, if one remains.
    PlayerId registerConnection();

    // Frees the color held by the given connection, if any, making it available again.
    void unregisterConnection(PlayerId id);

    // Returns the color assigned to the given connection, or std::nullopt if none was available.
    std::optional<char> colorFor(PlayerId id) const;

    // Records the username reported by a connection's "login" message.
    void setUsername(PlayerId id, std::string username);
    std::optional<std::string> usernameFor(PlayerId id) const;

    std::optional<std::string> usernameForColor(char color) const;

    // Reserves a logged-in player's color for graceDurationMs instead of freeing it immediately.
    void beginDisconnectGrace(PlayerId id, long long graceDurationMs);

    // Re-associates a reserved color with a reconnecting username, if one is pending.
    bool tryResumeSession(PlayerId newId, const std::string& username);

    // Frees and returns the colors of all disconnects whose grace period has elapsed.
    std::vector<char> extractExpiredDisconnects();

    // Registers a callback fired when a disconnect grace period begins, so the caller
    // can react (e.g. publish a bus event) without PlayerRegistry knowing about the bus.
    void setOnDisconnect(DisconnectHandler handler);

    // Registers a callback fired when a pending disconnect is resumed by a reconnect.
    void setOnReconnect(ReconnectHandler handler);

private:
    struct PlayerInfo {
        char color;
        std::string username;
    };
    struct PendingDisconnect {
        char color;
        std::string username;
        std::chrono::steady_clock::time_point expiresAt;
    };

    mutable std::mutex mutex_;
    std::vector<char> availableColors_;
    std::unordered_map<PlayerId, PlayerInfo> assignments_;
    std::vector<PendingDisconnect> pendingDisconnects_;
    PlayerId nextId_ = 1;
    DisconnectHandler onDisconnect_;
    ReconnectHandler onReconnect_;
};

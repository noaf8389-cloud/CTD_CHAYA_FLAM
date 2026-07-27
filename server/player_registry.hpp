#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using PlayerId = std::size_t;

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

private:
    struct PlayerInfo {
        char color;
        std::string username;
    };

    std::vector<char> availableColors_;
    std::unordered_map<PlayerId, PlayerInfo> assignments_;
    PlayerId nextId_ = 1;
};

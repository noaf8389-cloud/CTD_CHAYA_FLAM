#pragma once

#include <optional>
#include <string>

struct LoginResult {
    bool success;
    int rating;
};

class PlayerAccountStore {
public:
    virtual ~PlayerAccountStore() = default;

    // Verifies the password for an existing username, or creates a new account with the default rating if it doesn't exist yet.
    virtual LoginResult loginOrRegister(const std::string& username, const std::string& password) = 0;

    // Persists a new rating value for an existing account.
    virtual void updateRating(const std::string& username, int newRating) = 0;

    // Returns the account's current rating, or std::nullopt if the username doesn't exist.
    virtual std::optional<int> ratingFor(const std::string& username) const = 0;
};


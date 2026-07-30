#pragma once

#include <memory>
#include <mutex>

#include <pqxx/pqxx>

#include "player_account_store.hpp"

// PostgreSQL-backed implementation of PlayerAccountStore, for the cloud deployment.
// SqlitePlayerAccountStore remains the implementation used for local dev / Docker Compose.
class PostgresPlayerAccountStore : public PlayerAccountStore {
public:
    /// connectionString is a libpq connection string, e.g.
    /// "host=localhost port=5432 dbname=kungfuchess user=... password=...".
    /// Throws std::runtime_error if the connection fails or the players table can't be created —
    /// construct this once at startup, not per-request.
    explicit PostgresPlayerAccountStore(const std::string& connectionString);
    ~PostgresPlayerAccountStore() override;

    /// Same contract as any PlayerAccountStore: verifies the password for an existing
    /// username, or creates a new account with the default rating if it doesn't exist yet.
    /// Safe to call from multiple threads concurrently.
    LoginResult loginOrRegister(const std::string& username, const std::string& password) override;

    /// Persists newRating for an existing account. Does nothing if the username doesn't exist.
    void updateRating(const std::string& username, int newRating) override;

    /// Returns the account's current rating, or std::nullopt if the username doesn't exist.
    std::optional<int> ratingFor(const std::string& username) const override;

private:
    static constexpr int kDefaultRating = 1200;
    void createTableIfMissing();

    std::unique_ptr<pqxx::connection> connection_;
    // Guards connection_: a single pqxx::connection is not safe for concurrent use from
    // multiple threads, same reasoning as SqlitePlayerAccountStore's mutex.
    mutable std::mutex mutex_;
};

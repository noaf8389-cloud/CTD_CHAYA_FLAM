#pragma once

#include "player_account_store.hpp"

struct sqlite3;

class SqlitePlayerAccountStore : public PlayerAccountStore {
public:
    // Opens (or creates) a SQLite database file at dbPath and ensures the players table exists.
    explicit SqlitePlayerAccountStore(const std::string& dbPath);
    ~SqlitePlayerAccountStore() override;

    LoginResult loginOrRegister(const std::string& username, const std::string& password) override;
    void updateRating(const std::string& username, int newRating) override;
    std::optional<int> ratingFor(const std::string& username) const override;

private:
    static constexpr int kDefaultRating = 1200;
    void createTableIfMissing();

    sqlite3* db_;
};

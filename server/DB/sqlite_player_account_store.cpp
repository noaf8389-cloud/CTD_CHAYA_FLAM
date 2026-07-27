#include "sqlite_player_account_store.hpp"
#include "password_hasher.hpp"

#include <sqlite3.h>
#include <stdexcept>

SqlitePlayerAccountStore::SqlitePlayerAccountStore(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + dbPath);
    }
    createTableIfMissing();
}

SqlitePlayerAccountStore::~SqlitePlayerAccountStore() {
    sqlite3_close(db_);
}

void SqlitePlayerAccountStore::createTableIfMissing() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS players ("
        "  username TEXT PRIMARY KEY,"
        "  salt TEXT NOT NULL,"
        "  password_hash TEXT NOT NULL,"
        "  rating INTEGER NOT NULL DEFAULT 1200"
        ");";
    char* errorMessage = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        std::string message = errorMessage ? errorMessage : "unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("Cannot create players table: " + message);
    }
}

LoginResult SqlitePlayerAccountStore::loginOrRegister(const std::string& username, const std::string& password) {
    sqlite3_stmt* statement = nullptr;
    sqlite3_prepare_v2(db_, "SELECT salt, password_hash, rating FROM players WHERE username = ?;", -1, &statement, nullptr);
    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(statement) == SQLITE_ROW) {
        std::string salt = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        int rating = sqlite3_column_int(statement, 2);
        sqlite3_finalize(statement);

        bool matches = PasswordHasher::hash(password, salt) == storedHash;
        return LoginResult{matches, matches ? rating : 0};
    }
    sqlite3_finalize(statement);

    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hash(password, salt);

    sqlite3_stmt* insert = nullptr;
    sqlite3_prepare_v2(db_, "INSERT INTO players (username, salt, password_hash, rating) VALUES (?, ?, ?, ?);", -1, &insert, nullptr);
    sqlite3_bind_text(insert, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert, 2, salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert, 3, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insert, 4, kDefaultRating);
    sqlite3_step(insert);
    sqlite3_finalize(insert);

    return LoginResult{true, kDefaultRating};
}

void SqlitePlayerAccountStore::updateRating(const std::string& username, int newRating) {
    sqlite3_stmt* statement = nullptr;
    sqlite3_prepare_v2(db_, "UPDATE players SET rating = ? WHERE username = ?;", -1, &statement, nullptr);
    sqlite3_bind_int(statement, 1, newRating);
    sqlite3_bind_text(statement, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

std::optional<int> SqlitePlayerAccountStore::ratingFor(const std::string& username) const {
    sqlite3_stmt* statement = nullptr;
    sqlite3_prepare_v2(db_, "SELECT rating FROM players WHERE username = ?;", -1, &statement, nullptr);
    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<int> result;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        result = sqlite3_column_int(statement, 0);
    }
    sqlite3_finalize(statement);
    return result;
}

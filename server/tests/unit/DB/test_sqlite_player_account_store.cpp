#include <filesystem>
#include "../../catch2/catch_amalgamated.hpp"
#include "../../../db/sqlite_player_account_store.hpp"

TEST_CASE("a new username is registered with the default rating") {
    SqlitePlayerAccountStore store(":memory:");
    LoginResult result = store.loginOrRegister("noa", "pw123");
    REQUIRE(result.success);
    REQUIRE(result.rating == 1200);
}

TEST_CASE("logging in again with the correct password succeeds") {
    SqlitePlayerAccountStore store(":memory:");
    store.loginOrRegister("noa", "pw123");
    LoginResult result = store.loginOrRegister("noa", "pw123");
    REQUIRE(result.success);
}

TEST_CASE("logging in with the wrong password fails") {
    SqlitePlayerAccountStore store(":memory:");
    store.loginOrRegister("noa", "pw123");
    LoginResult result = store.loginOrRegister("noa", "wrong");
    REQUIRE(result.success == false);
}

TEST_CASE("updateRating persists and is reflected in ratingFor") {
    SqlitePlayerAccountStore store(":memory:");
    store.loginOrRegister("noa", "pw123");
    store.updateRating("noa", 1300);
    REQUIRE(store.ratingFor("noa").value() == 1300);
}

TEST_CASE("ratingFor returns nullopt for an unknown username") {
    SqlitePlayerAccountStore store(":memory:");
    REQUIRE(store.ratingFor("ghost").has_value() == false);
}

TEST_CASE("usernames are case-sensitive — different accounts") {
    SqlitePlayerAccountStore store(":memory:");
    store.loginOrRegister("noa", "pw123");
    LoginResult result = store.loginOrRegister("Noa", "pw123");

    REQUIRE(result.success);
    REQUIRE(result.rating == 1200);   // new account, not the same as "noa"
}

TEST_CASE("ratings survive across store instances when backed by a real file") {
    std::string dbPath = (std::filesystem::temp_directory_path() / "kfc_test_accounts.db").string();
    std::filesystem::remove(dbPath);

    {
        SqlitePlayerAccountStore store(dbPath);
        store.loginOrRegister("noa", "pw123");
        store.updateRating("noa", 1350);
    }

    {
        SqlitePlayerAccountStore reopened(dbPath);
        REQUIRE(reopened.ratingFor("noa").value() == 1350);
    }

    std::filesystem::remove(dbPath);
}

TEST_CASE("supports non-ASCII usernames and passwords") {
    SqlitePlayerAccountStore store(":memory:");
    LoginResult first = store.loginOrRegister(u8"\u05e0\u05d5\u05e2\u05d4", u8"\u05e1\u05d9\u05e1\u05de\u05d4");
    LoginResult again = store.loginOrRegister(u8"\u05e0\u05d5\u05e2\u05d4", u8"\u05e1\u05d9\u05e1\u05de\u05d4");
    LoginResult wrong = store.loginOrRegister(u8"\u05e0\u05d5\u05e2\u05d4", "wrong");

    REQUIRE(first.success);
    REQUIRE(again.success);
    REQUIRE(wrong.success == false);
}

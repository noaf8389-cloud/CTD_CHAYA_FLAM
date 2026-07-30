#include "../../catch2/catch_amalgamated.hpp"
#include "../../../db/postgres_player_account_store.hpp"

namespace {
    // Real, containerized PostgreSQL — see docker-compose.yml at the repo root.
    const std::string kConnectionString = "host=postgres port=5432 dbname=kungfuchess user=kungfuchess password=devpassword";
}

TEST_CASE("PostgresPlayerAccountStore: a new username is registered with the default rating") {
    PostgresPlayerAccountStore store(kConnectionString);
    LoginResult result = store.loginOrRegister("pg_test_new_account", "pw123");
    REQUIRE(result.success);
    REQUIRE(result.rating == 1200);
}

TEST_CASE("PostgresPlayerAccountStore: logging in again with the correct password succeeds") {
    PostgresPlayerAccountStore store(kConnectionString);
    store.loginOrRegister("pg_test_correct_password", "pw123");
    LoginResult result = store.loginOrRegister("pg_test_correct_password", "pw123");
    REQUIRE(result.success);
}

TEST_CASE("PostgresPlayerAccountStore: logging in with the wrong password fails") {
    PostgresPlayerAccountStore store(kConnectionString);
    store.loginOrRegister("pg_test_wrong_password", "correct");
    LoginResult result = store.loginOrRegister("pg_test_wrong_password", "wrong");
    REQUIRE(result.success == false);
}

TEST_CASE("PostgresPlayerAccountStore: updateRating persists and is reflected in ratingFor") {
    PostgresPlayerAccountStore store(kConnectionString);
    store.loginOrRegister("pg_test_update_rating", "pw123");
    store.updateRating("pg_test_update_rating", 1300);
    REQUIRE(store.ratingFor("pg_test_update_rating").value() == 1300);
}

TEST_CASE("PostgresPlayerAccountStore: ratingFor returns nullopt for an unknown username") {
    PostgresPlayerAccountStore store(kConnectionString);
    REQUIRE(store.ratingFor("pg_test_definitely_unknown_user").has_value() == false);
}

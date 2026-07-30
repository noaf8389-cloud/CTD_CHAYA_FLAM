#include "../../catch2/catch_amalgamated.hpp"
#include "../../../lobby/in_memory_lobby_store.hpp"

TEST_CASE("nextConnectionId returns a distinct id each call") {
    InMemoryLobbyStore store;
    auto first = store.nextConnectionId();
    auto second = store.nextConnectionId();
    REQUIRE(first != second);
}

TEST_CASE("hasConnection is false until addConnection is called") {
    InMemoryLobbyStore store;
    auto id = store.nextConnectionId();
    REQUIRE(store.hasConnection(id) == false);

    store.addConnection(id);
    REQUIRE(store.hasConnection(id) == true);
}

TEST_CASE("identityFor is nullopt before setIdentity is called") {
    InMemoryLobbyStore store;
    auto id = store.nextConnectionId();
    store.addConnection(id);

    REQUIRE(store.identityFor(id).has_value() == false);
}

TEST_CASE("setIdentity records username and rating") {
    InMemoryLobbyStore store;
    auto id = store.nextConnectionId();
    store.addConnection(id);

    store.setIdentity(id, "noa", 1300);

    auto identity = store.identityFor(id);
    REQUIRE(identity.has_value());
    REQUIRE(identity->username == "noa");
    REQUIRE(identity->rating == 1300);
}

TEST_CASE("setIdentity overwrites a previous identity for the same connection") {
    InMemoryLobbyStore store;
    auto id = store.nextConnectionId();
    store.addConnection(id);

    store.setIdentity(id, "noa", 1300);
    store.setIdentity(id, "noa", 1350);

    REQUIRE(store.identityFor(id)->rating == 1350);
}

TEST_CASE("removeConnection clears both connection membership and identity") {
    InMemoryLobbyStore store;
    auto id = store.nextConnectionId();
    store.addConnection(id);
    store.setIdentity(id, "noa", 1300);

    store.removeConnection(id);

    REQUIRE(store.hasConnection(id) == false);
    REQUIRE(store.identityFor(id).has_value() == false);
}

TEST_CASE("identityFor and hasConnection return empty/false for an id that was never added") {
    InMemoryLobbyStore store;
    REQUIRE(store.hasConnection(999) == false);
    REQUIRE(store.identityFor(999).has_value() == false);
}

TEST_CASE("two connections hold independent identities") {
    InMemoryLobbyStore store;
    auto first = store.nextConnectionId();
    auto second = store.nextConnectionId();
    store.addConnection(first);
    store.addConnection(second);

    store.setIdentity(first, "noa", 1300);
    store.setIdentity(second, "chaya", 1500);

    REQUIRE(store.identityFor(first)->username == "noa");
    REQUIRE(store.identityFor(second)->username == "chaya");
}

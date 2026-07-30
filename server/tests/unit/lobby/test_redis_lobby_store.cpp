#include "../../catch2/catch_amalgamated.hpp"
#include "../../../lobby/redis_lobby_store.hpp"

namespace {
    // Real, containerized Redis — see docker-compose.yml at the repo root.
    RedisLobbyStore makeStore() { return RedisLobbyStore("redis", 6379); }
}

TEST_CASE("RedisLobbyStore: nextConnectionId returns a distinct id each call") {
    auto store = makeStore();
    auto first = store.nextConnectionId();
    auto second = store.nextConnectionId();
    REQUIRE(first != second);
}

TEST_CASE("RedisLobbyStore: hasConnection is false until addConnection is called") {
    auto store = makeStore();
    auto id = store.nextConnectionId();
    REQUIRE(store.hasConnection(id) == false);

    store.addConnection(id);
    REQUIRE(store.hasConnection(id) == true);
}

TEST_CASE("RedisLobbyStore: identityFor is nullopt before setIdentity is called") {
    auto store = makeStore();
    auto id = store.nextConnectionId();
    store.addConnection(id);

    REQUIRE(store.identityFor(id).has_value() == false);
}

TEST_CASE("RedisLobbyStore: setIdentity records username and rating") {
    auto store = makeStore();
    auto id = store.nextConnectionId();
    store.addConnection(id);

    store.setIdentity(id, "noa", 1300);

    auto identity = store.identityFor(id);
    REQUIRE(identity.has_value());
    REQUIRE(identity->username == "noa");
    REQUIRE(identity->rating == 1300);
}

TEST_CASE("RedisLobbyStore: setIdentity overwrites a previous identity for the same connection") {
    auto store = makeStore();
    auto id = store.nextConnectionId();
    store.addConnection(id);

    store.setIdentity(id, "noa", 1300);
    store.setIdentity(id, "noa", 1350);

    REQUIRE(store.identityFor(id)->rating == 1350);
}

TEST_CASE("RedisLobbyStore: removeConnection clears both connection membership and identity") {
    auto store = makeStore();
    auto id = store.nextConnectionId();
    store.addConnection(id);
    store.setIdentity(id, "noa", 1300);

    store.removeConnection(id);

    REQUIRE(store.hasConnection(id) == false);
    REQUIRE(store.identityFor(id).has_value() == false);
}

TEST_CASE("RedisLobbyStore: two connections hold independent identities") {
    auto store = makeStore();
    auto first = store.nextConnectionId();
    auto second = store.nextConnectionId();
    store.addConnection(first);
    store.addConnection(second);

    store.setIdentity(first, "noa", 1300);
    store.setIdentity(second, "chaya", 1500);

    REQUIRE(store.identityFor(first)->username == "noa");
    REQUIRE(store.identityFor(second)->username == "chaya");
}

#include "../catch2/catch_amalgamated.hpp"
#include "../../lobby/lobby_registry.hpp"
#include "../../lobby/in_memory_lobby_store.hpp"

TEST_CASE("registerConnection returns an id that is not yet identified") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();
    REQUIRE(lobby.isIdentified(id) == false);
}

TEST_CASE("registerConnection returns a distinct id for each connection") {
    LobbyRegistry lobby;
    auto first = lobby.registerConnection();
    auto second = lobby.registerConnection();
    REQUIRE(first != second);
}

TEST_CASE("identify records username and rating for a registered connection") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();

    lobby.identify(id, "noa", 1300);

    REQUIRE(lobby.isIdentified(id));
    REQUIRE(lobby.usernameFor(id).value() == "noa");
    REQUIRE(lobby.ratingFor(id).value() == 1300);
}

TEST_CASE("identify overwrites a previous identity for the same connection") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();

    lobby.identify(id, "noa", 1300);
    lobby.identify(id, "noa", 1350);

    REQUIRE(lobby.usernameFor(id).value() == "noa");
    REQUIRE(lobby.ratingFor(id).value() == 1350);
}

TEST_CASE("usernameFor and ratingFor return nullopt before identify is called") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();

    REQUIRE(lobby.usernameFor(id).has_value() == false);
    REQUIRE(lobby.ratingFor(id).has_value() == false);
}

TEST_CASE("usernameFor and ratingFor return nullopt for an id that was never registered") {
    LobbyRegistry lobby;
    REQUIRE(lobby.usernameFor(999).has_value() == false);
    REQUIRE(lobby.ratingFor(999).has_value() == false);
}

TEST_CASE("isIdentified returns false for an id that was never registered") {
    LobbyRegistry lobby;
    REQUIRE(lobby.isIdentified(999) == false);
}

TEST_CASE("unregisterConnection removes the connection and its identity") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();
    lobby.identify(id, "noa", 1300);

    lobby.unregisterConnection(id);

    REQUIRE(lobby.isIdentified(id) == false);
    REQUIRE(lobby.usernameFor(id).has_value() == false);
    REQUIRE(lobby.ratingFor(id).has_value() == false);
}

TEST_CASE("unregisterConnection on a never-identified connection does nothing harmful") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();

    REQUIRE_NOTHROW(lobby.unregisterConnection(id));
    REQUIRE(lobby.isIdentified(id) == false);
}

TEST_CASE("unregistering an unknown lobby connection id does nothing") {
    LobbyRegistry lobby;
    REQUIRE_NOTHROW(lobby.unregisterConnection(12345));
}

TEST_CASE("unregistering the same id twice does nothing on the second call") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();
    lobby.unregisterConnection(id);

    REQUIRE_NOTHROW(lobby.unregisterConnection(id));
}

TEST_CASE("identify on an unknown id does nothing") {
    LobbyRegistry lobby;
    lobby.identify(999, "ghost", 1000);
    REQUIRE(lobby.isIdentified(999) == false);
}

TEST_CASE("identify on an unregistered (already removed) id does nothing") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();
    lobby.unregisterConnection(id);

    lobby.identify(id, "noa", 1300);

    REQUIRE(lobby.isIdentified(id) == false);
}

TEST_CASE("two different connections can hold independent identities") {
    LobbyRegistry lobby;
    auto first = lobby.registerConnection();
    auto second = lobby.registerConnection();

    lobby.identify(first, "noa", 1300);
    lobby.identify(second, "chaya", 1500);

    REQUIRE(lobby.usernameFor(first).value() == "noa");
    REQUIRE(lobby.usernameFor(second).value() == "chaya");
    REQUIRE(lobby.ratingFor(first).value() == 1300);
    REQUIRE(lobby.ratingFor(second).value() == 1500);
}

TEST_CASE("setOnIdentified fires with the id, username and rating when identify succeeds") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();

    LobbyRegistry::ConnectionId calledId = 0;
    std::string calledUsername;
    int calledRating = 0;
    lobby.setOnIdentified([&](LobbyRegistry::ConnectionId connectionId, const std::string& username, int rating) {
        calledId = connectionId;
        calledUsername = username;
        calledRating = rating;
    });

    lobby.identify(id, "noa", 1300);

    REQUIRE(calledId == id);
    REQUIRE(calledUsername == "noa");
    REQUIRE(calledRating == 1300);
}

TEST_CASE("setOnIdentified fires again on a repeated identify for the same connection") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();

    int callCount = 0;
    lobby.setOnIdentified([&](LobbyRegistry::ConnectionId, const std::string&, int) { callCount++; });

    lobby.identify(id, "noa", 1300);
    lobby.identify(id, "noa", 1350);

    REQUIRE(callCount == 2);
}

TEST_CASE("setOnIdentified does not fire for an unknown id") {
    LobbyRegistry lobby;

    bool called = false;
    lobby.setOnIdentified([&](LobbyRegistry::ConnectionId, const std::string&, int) { called = true; });

    lobby.identify(999, "ghost", 1000);

    REQUIRE(called == false);
}

TEST_CASE("setOnIdentified does not fire when no handler was set") {
    LobbyRegistry lobby;
    auto id = lobby.registerConnection();
    REQUIRE_NOTHROW(lobby.identify(id, "noa", 1300));
}

TEST_CASE("LobbyRegistry constructed with an external LobbyStore delegates to it") {
    InMemoryLobbyStore store;
    LobbyRegistry lobby(store);

    auto id = lobby.registerConnection();
    lobby.identify(id, "noa", 1300);

    REQUIRE(lobby.usernameFor(id).value() == "noa");
    REQUIRE(store.identityFor(id).has_value());
    REQUIRE(store.identityFor(id)->username == "noa");
}

#include "../catch2/catch_amalgamated.hpp"
#include "../../player_registry.hpp"

TEST_CASE("the first connection is assigned white") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    REQUIRE(registry.colorFor(id).value() == 'w');
}

TEST_CASE("the second connection is assigned black") {
    PlayerRegistry registry;
    registry.registerConnection();
    PlayerId second = registry.registerConnection();
    REQUIRE(registry.colorFor(second).value() == 'b');
}

TEST_CASE("a third connection gets no color once both are taken") {
    PlayerRegistry registry;
    registry.registerConnection();
    registry.registerConnection();
    PlayerId third = registry.registerConnection();
    REQUIRE(registry.colorFor(third).has_value() == false);
}

TEST_CASE("colorFor returns nullopt for an id that was never registered") {
    PlayerRegistry registry;
    REQUIRE(registry.colorFor(999).has_value() == false);
}

TEST_CASE("unregistering a connection frees its color for reuse") {
    PlayerRegistry registry;
    PlayerId first = registry.registerConnection();
    registry.registerConnection();

    registry.unregisterConnection(first);
    PlayerId newcomer = registry.registerConnection();

    REQUIRE(registry.colorFor(newcomer).value() == 'w');
}

TEST_CASE("unregistering an unknown id does nothing") {
    PlayerRegistry registry;
    REQUIRE_NOTHROW(registry.unregisterConnection(12345));
}

TEST_CASE("a custom color list is honored in order") {
    PlayerRegistry registry(std::vector<char>{'r', 'g', 'b'});
    PlayerId first = registry.registerConnection();
    PlayerId second = registry.registerConnection();
    PlayerId third = registry.registerConnection();

    REQUIRE(registry.colorFor(first).value() == 'r');
    REQUIRE(registry.colorFor(second).value() == 'g');
    REQUIRE(registry.colorFor(third).value() == 'b');
}

TEST_CASE("usernameFor returns nullopt before a login message arrives") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    REQUIRE(registry.usernameFor(id).has_value() == false);
}

TEST_CASE("setUsername records the username for an assigned connection") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    registry.setUsername(id, "noa");
    REQUIRE(registry.usernameFor(id).value() == "noa");
}

TEST_CASE("setUsername on an unassigned (spectator) connection does nothing") {
    PlayerRegistry registry;
    registry.registerConnection();
    registry.registerConnection();
    PlayerId spectator = registry.registerConnection();   // no color left

    registry.setUsername(spectator, "noa");
    REQUIRE(registry.usernameFor(spectator).has_value() == false);
}

TEST_CASE("setUsername on an unknown id does nothing") {
    PlayerRegistry registry;
    REQUIRE_NOTHROW(registry.setUsername(999, "noa"));
}

TEST_CASE("usernameFor returns nullopt after the connection is unregistered") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    registry.setUsername(id, "noa");

    registry.unregisterConnection(id);
    REQUIRE(registry.usernameFor(id).has_value() == false);
}

TEST_CASE("usernameForColor returns the username of whoever holds that color") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    registry.setUsername(id, "noa");

    REQUIRE(registry.usernameForColor('w').value() == "noa");
}

TEST_CASE("usernameForColor returns nullopt when no one holds that color yet") {
    PlayerRegistry registry;
    REQUIRE(registry.usernameForColor('w').has_value() == false);
}

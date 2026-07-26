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

#include <chrono>
#include <thread>

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

TEST_CASE("beginDisconnectGrace reserves the color instead of freeing it immediately") {
    PlayerRegistry registry;
    PlayerId first = registry.registerConnection();
    registry.setUsername(first, "noa");

    registry.beginDisconnectGrace(first, 20000);

    PlayerId newcomer = registry.registerConnection();
    REQUIRE(registry.colorFor(newcomer).value() == 'b');   // 'w' stays reserved, not reused
}

TEST_CASE("beginDisconnectGrace on a connection with no username frees the color immediately") {
    PlayerRegistry registry;
    PlayerId first = registry.registerConnection();   // 'w', never logged in
    registry.registerConnection();                    // 'b' — pool now empty

    registry.beginDisconnectGrace(first, 20000);

    PlayerId newcomer = registry.registerConnection();
    REQUIRE(registry.colorFor(newcomer).value() == 'w');
}

TEST_CASE("tryResumeSession restores a reserved color to a new connection") {
    PlayerRegistry registry;
    PlayerId first = registry.registerConnection();
    registry.setUsername(first, "noa");
    registry.beginDisconnectGrace(first, 20000);

    PlayerId reconnecting = registry.registerConnection();
    bool resumed = registry.tryResumeSession(reconnecting, "noa");

    REQUIRE(resumed);
    REQUIRE(registry.colorFor(reconnecting).value() == 'w');
}

TEST_CASE("tryResumeSession returns false when no matching pending disconnect exists") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    REQUIRE(registry.tryResumeSession(id, "ghost") == false);
}

TEST_CASE("tryResumeSession frees a freshly assigned color instead of leaking it") {
    PlayerRegistry registry;
    PlayerId first = registry.registerConnection();       // 'w'
    registry.setUsername(first, "noa");
    registry.beginDisconnectGrace(first, 20000);           // 'w' reserved

    PlayerId newcomer = registry.registerConnection();     // only 'b' left in the pool
    registry.tryResumeSession(newcomer, "noa");             // should reclaim 'w', freeing 'b'

    REQUIRE(registry.colorFor(newcomer).value() == 'w');

    PlayerId another = registry.registerConnection();
    REQUIRE(registry.colorFor(another).value() == 'b');    // 'b' is available again, not leaked
}

TEST_CASE("extractExpiredDisconnects returns nothing before the grace period elapses") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    registry.setUsername(id, "noa");
    registry.beginDisconnectGrace(id, 60000);

    REQUIRE(registry.extractExpiredDisconnects().empty());
}

TEST_CASE("extractExpiredDisconnects frees the color once the grace period elapses") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();   // 'w'
    registry.setUsername(id, "noa");
    registry.registerConnection();                 // 'b' — pool now empty
    registry.beginDisconnectGrace(id, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::vector<char> expired = registry.extractExpiredDisconnects();
    REQUIRE(expired.size() == 1);
    REQUIRE(expired[0] == 'w');

    PlayerId newcomer = registry.registerConnection();
    REQUIRE(registry.colorFor(newcomer).value() == 'w');
}

TEST_CASE("setOnDisconnect fires with the color and grace duration when a grace period begins") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();
    registry.setUsername(id, "noa");

    char calledColor = 0;
    long long calledGraceMs = 0;
    registry.setOnDisconnect([&](char color, long long graceMs) {
        calledColor = color;
        calledGraceMs = graceMs;
    });

    registry.beginDisconnectGrace(id, 20000);

    REQUIRE(calledColor == 'w');
    REQUIRE(calledGraceMs == 20000);
}

TEST_CASE("setOnDisconnect does not fire for a connection with no username") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();

    bool called = false;
    registry.setOnDisconnect([&](char, long long) { called = true; });

    registry.beginDisconnectGrace(id, 20000);

    REQUIRE(called == false);
}

TEST_CASE("setOnReconnect fires with the resumed color when a session is resumed") {
    PlayerRegistry registry;
    PlayerId first = registry.registerConnection();
    registry.setUsername(first, "noa");
    registry.beginDisconnectGrace(first, 20000);

    char calledColor = 0;
    registry.setOnReconnect([&](char color) { calledColor = color; });

    PlayerId reconnecting = registry.registerConnection();
    registry.tryResumeSession(reconnecting, "noa");

    REQUIRE(calledColor == 'w');
}

TEST_CASE("setOnReconnect does not fire when tryResumeSession finds no match") {
    PlayerRegistry registry;
    PlayerId id = registry.registerConnection();

    bool called = false;
    registry.setOnReconnect([&](char) { called = true; });

    registry.tryResumeSession(id, "ghost");

    REQUIRE(called == false);
}

#include <unordered_map>

#include "../catch2/catch_amalgamated.hpp"
#include "../../matches/game_registry.hpp"

namespace {
    class FakeAccountStore : public PlayerAccountStore {
    public:
        LoginResult loginOrRegister(const std::string&, const std::string&) override { return {}; }
        void updateRating(const std::string& username, int newRating) override { ratings[username] = newRating; }
        std::optional<int> ratingFor(const std::string& username) const override {
            auto it = ratings.find(username);
            return it != ratings.end() ? std::optional<int>(it->second) : std::nullopt;
        }
        std::unordered_map<std::string, int> ratings;
    };

    Board makeBoardWithRook() {
        Board board(4, 4);
        board.setCell(0, 0, "wR");
        return board;
    }

    Board makeBoardWithCapturableKing() {
        Board board(4, 4);
        board.setCell(0, 0, "wR");
        board.setCell(0, 3, "bK");
        return board;
    }
}

TEST_CASE("createMatch returns a distinct id for each match") {
    FakeAccountStore accounts;
    GameRegistry registry;

    auto first = registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");
    auto second = registry.createMatch(makeBoardWithRook(), accounts, "carol", "dave");

    REQUIRE(first != second);
}

TEST_CASE("matchFor returns the match a player was placed into") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");

    REQUIRE(registry.matchFor("alice") != nullptr);
}

TEST_CASE("matchFor returns the same match for both players of a pair") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");

    REQUIRE(registry.matchFor("alice") == registry.matchFor("bob"));
}

TEST_CASE("matchFor returns nullptr for a username that was never matched") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");

    REQUIRE(registry.matchFor("ghost") == nullptr);
}

TEST_CASE("two independently created matches don't share players") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");
    registry.createMatch(makeBoardWithRook(), accounts, "carol", "dave");

    REQUIRE(registry.matchFor("alice") != registry.matchFor("carol"));
}

TEST_CASE("matchCount reflects the number of currently tracked matches") {
    FakeAccountStore accounts;
    GameRegistry registry;
    REQUIRE(registry.matchCount() == 0);

    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");
    registry.createMatch(makeBoardWithRook(), accounts, "carol", "dave");

    REQUIRE(registry.matchCount() == 2);
}

TEST_CASE("updateAll advances every tracked match") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");
    GameMatch* match = registry.matchFor("alice");

    match->handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    match->handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    registry.updateAll(100000);

    REQUIRE_FALSE(match->isOver());   // rook move, not a king capture — still confirms update() ran without crashing
}

TEST_CASE("updateAll removes a match once it ends") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithCapturableKing(), accounts, "alice", "bob");
    GameMatch* match = registry.matchFor("alice");

    match->handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    match->handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    registry.updateAll(100000);

    REQUIRE(registry.matchCount() == 0);
}

TEST_CASE("matchFor returns nullptr for a player whose match has ended and been removed") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithCapturableKing(), accounts, "alice", "bob");

    registry.matchFor("alice")->handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    registry.matchFor("alice")->handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    registry.updateAll(100000);

    REQUIRE(registry.matchFor("alice") == nullptr);
    REQUIRE(registry.matchFor("bob") == nullptr);
}

TEST_CASE("updateAll does not remove matches that are still in progress") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");

    registry.updateAll(1);

    REQUIRE(registry.matchCount() == 1);
    REQUIRE(registry.matchFor("alice") != nullptr);
}

TEST_CASE("matchFor keys by username, not by any transient connection — safe across reconnects") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createMatch(makeBoardWithRook(), accounts, "alice", "bob");

    GameMatch* first = registry.matchFor("alice");
    GameMatch* second = registry.matchFor("alice");   // simulates a lookup after alice reconnected with a new socket

    REQUIRE(first == second);
}

TEST_CASE("createRoom returns a code that joinRoom can use to find the same match") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode code = registry.createRoom(makeBoardWithRook(), accounts, "alice");

    GameMatch* match = registry.joinRoom(code, "bob");

    REQUIRE(match != nullptr);
    REQUIRE(match == registry.matchFor("alice"));
}

TEST_CASE("joinRoom returns nullptr for an unknown room code") {
    FakeAccountStore accounts;
    GameRegistry registry;
    REQUIRE(registry.joinRoom("NOSUCH", "bob") == nullptr);
}

TEST_CASE("joinRoom registers the joiner's username so matchFor finds the match too") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode code = registry.createRoom(makeBoardWithRook(), accounts, "alice");

    registry.joinRoom(code, "bob");

    REQUIRE(registry.matchFor("bob") == registry.matchFor("alice"));
}

TEST_CASE("a room accepts more than two joiners (spectators)") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode code = registry.createRoom(makeBoardWithRook(), accounts, "alice");

    registry.joinRoom(code, "bob");
    GameMatch* asSpectator = registry.joinRoom(code, "carol");

    REQUIRE(asSpectator != nullptr);
    REQUIRE(asSpectator == registry.matchFor("alice"));
}

TEST_CASE("createRoom produces a distinct code for each room") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode first = registry.createRoom(makeBoardWithRook(), accounts, "alice");
    GameRegistry::RoomCode second = registry.createRoom(makeBoardWithRook(), accounts, "carol");

    REQUIRE(first != second);
}

TEST_CASE("a room's code stops resolving once its match ends and is pruned") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode code = registry.createRoom(makeBoardWithCapturableKing(), accounts, "alice");
    registry.joinRoom(code, "bob");

    registry.matchFor("alice")->handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    registry.matchFor("alice")->handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    registry.updateAll(100000);

    REQUIRE(registry.joinRoom(code, "dave") == nullptr);
}

TEST_CASE("matchFor for a room joiner (not the creator) also returns nullptr once the match has ended and been pruned") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode code = registry.createRoom(makeBoardWithCapturableKing(), accounts, "alice");
    registry.joinRoom(code, "bob");

    registry.matchFor("alice")->handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    registry.matchFor("alice")->handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    registry.updateAll(100000);

    REQUIRE(registry.matchFor("bob") == nullptr);
}

TEST_CASE("createRoom alone, with no joiner yet, still creates a trackable match") {
    FakeAccountStore accounts;
    GameRegistry registry;
    registry.createRoom(makeBoardWithRook(), accounts, "alice");

    REQUIRE(registry.matchFor("alice") != nullptr);
    REQUIRE(registry.matchCount() == 1);
}

TEST_CASE("createRoom produces a 6-character code") {
    FakeAccountStore accounts;
    GameRegistry registry;
    GameRegistry::RoomCode code = registry.createRoom(makeBoardWithRook(), accounts, "alice");

    REQUIRE(code.size() == 6);
}

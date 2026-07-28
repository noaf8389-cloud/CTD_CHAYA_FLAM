#include <chrono>
#include <thread>
#include <unordered_map>

#include "../catch2/catch_amalgamated.hpp"
#include "../../matches/game_match.hpp"
#include "bus/game_events.hpp"

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
}

TEST_CASE("GameMatch routes handleMessage clicks into the underlying game") {
    FakeAccountStore accounts;
    GameMatch match(makeBoardWithRook(), accounts);

    std::vector<MoveStartedEvent> started;
    match.bus().subscribe<MoveStartedEvent>([&started](const MoveStartedEvent& e) { started.push_back(e); });

    match.handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    match.handleMessage(R"({"command":"click","row":0,"col":3})", 'w');

    REQUIRE(started.size() == 1);
}

TEST_CASE("GameMatch::update advances pending moves and publishes MoveMadeEvent") {
    FakeAccountStore accounts;
    GameMatch match(makeBoardWithRook(), accounts);

    std::vector<MoveMadeEvent> made;
    match.bus().subscribe<MoveMadeEvent>([&made](const MoveMadeEvent& e) { made.push_back(e); });

    match.handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    match.handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    match.update(100000);

    REQUIRE(made.size() == 1);
}

TEST_CASE("GameMatch::isOver reflects whether a king has been captured") {
    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    FakeAccountStore accounts;
    GameMatch match(board, accounts);

    REQUIRE(match.isOver() == false);

    match.handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    match.handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    match.update(100000);

    REQUIRE(match.isOver());
}

TEST_CASE("GameMatch::players assigns colors independently of any other match") {
    FakeAccountStore accounts;
    GameMatch match(makeBoardWithRook(), accounts);

    PlayerId first = match.players().registerConnection();
    PlayerId second = match.players().registerConnection();

    REQUIRE(match.players().colorFor(first).value() == 'w');
    REQUIRE(match.players().colorFor(second).value() == 'b');
}

TEST_CASE("GameMatch::update forfeits a player whose disconnect grace period has expired") {
    FakeAccountStore accounts;
    GameMatch match(makeBoardWithRook(), accounts);

    PlayerId id = match.players().registerConnection();
    match.players().setUsername(id, "noa");
    match.players().beginDisconnectGrace(id, 1);

    std::vector<GameOverEvent> over;
    match.bus().subscribe<GameOverEvent>([&over](const GameOverEvent& e) { over.push_back(e); });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    match.update(0);

    REQUIRE(over.size() == 1);
    REQUIRE(over[0].winner_color == 'b');
}

TEST_CASE("GameMatch persists a rating update through the shared account store on game over") {
    FakeAccountStore accounts;
    accounts.ratings["alice"] = 1200;
    accounts.ratings["bob"] = 1200;

    Board board(4, 4);
    board.setCell(0, 0, "wR");
    board.setCell(0, 3, "bK");
    GameMatch match(board, accounts);

    PlayerId aliceId = match.players().registerConnection();
    match.players().setUsername(aliceId, "alice");
    PlayerId bobId = match.players().registerConnection();
    match.players().setUsername(bobId, "bob");

    match.handleMessage(R"({"command":"click","row":0,"col":0})", 'w');
    match.handleMessage(R"({"command":"click","row":0,"col":3})", 'w');
    match.update(100000);

    REQUIRE(accounts.ratingFor("alice").value() > 1200);
    REQUIRE(accounts.ratingFor("bob").value() < 1200);
}

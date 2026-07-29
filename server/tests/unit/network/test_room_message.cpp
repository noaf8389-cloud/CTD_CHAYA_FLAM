#include "../../catch2/catch_amalgamated.hpp"
#include "../../../network/room_message.hpp"

TEST_CASE("parseCreateRoomRequest returns true for a valid createRoom command") {
    REQUIRE(parseCreateRoomRequest(R"({"command":"createRoom"})"));
}

TEST_CASE("parseCreateRoomRequest returns false for a different command") {
    REQUIRE_FALSE(parseCreateRoomRequest(R"({"command":"joinRoom","roomCode":"ABC123"})"));
}

TEST_CASE("parseCreateRoomRequest returns false for malformed JSON") {
    REQUIRE_FALSE(parseCreateRoomRequest("not valid json"));
}

TEST_CASE("parseCreateRoomRequest returns false when command has the wrong JSON type") {
    REQUIRE_FALSE(parseCreateRoomRequest(R"({"command":42})"));
}

TEST_CASE("parseCreateRoomRequest is case-sensitive about the command name") {
    REQUIRE_FALSE(parseCreateRoomRequest(R"({"command":"CreateRoom"})"));
}

TEST_CASE("parseCreateRoomRequest ignores unrelated extra fields") {
    REQUIRE(parseCreateRoomRequest(R"({"command":"createRoom","extra":true})"));
}

TEST_CASE("parseJoinRoomRequest extracts the room code from a valid joinRoom command") {
    auto result = parseJoinRoomRequest(R"({"command":"joinRoom","roomCode":"ABC123"})");
    REQUIRE(result->roomCode == "ABC123");
}

TEST_CASE("parseJoinRoomRequest returns nullopt for a non-joinRoom command") {
    auto result = parseJoinRoomRequest(R"({"command":"createRoom"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseJoinRoomRequest returns nullopt for malformed JSON") {
    REQUIRE(parseJoinRoomRequest("not valid json").has_value() == false);
}

TEST_CASE("parseJoinRoomRequest returns nullopt when roomCode is missing") {
    auto result = parseJoinRoomRequest(R"({"command":"joinRoom"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseJoinRoomRequest returns nullopt when roomCode is empty") {
    auto result = parseJoinRoomRequest(R"({"command":"joinRoom","roomCode":""})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseJoinRoomRequest returns nullopt instead of throwing when roomCode has the wrong JSON type") {
    auto result = parseJoinRoomRequest(R"({"command":"joinRoom","roomCode":123})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseJoinRoomRequest returns nullopt instead of throwing when command has the wrong JSON type") {
    auto result = parseJoinRoomRequest(R"({"command":42,"roomCode":"ABC123"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseJoinRoomRequest is case-sensitive about the command name") {
    auto result = parseJoinRoomRequest(R"({"command":"JoinRoom","roomCode":"ABC123"})");
    REQUIRE(result.has_value() == false);
}

TEST_CASE("parseJoinRoomRequest ignores unrelated extra fields") {
    auto result = parseJoinRoomRequest(R"({"command":"joinRoom","roomCode":"ABC123","extra":true})");
    REQUIRE(result->roomCode == "ABC123");
}

TEST_CASE("parseFindMatchRequest returns true for a valid findMatch command") {
    REQUIRE(parseFindMatchRequest(R"({"command":"findMatch"})"));
}

TEST_CASE("parseFindMatchRequest returns false for a different command") {
    REQUIRE_FALSE(parseFindMatchRequest(R"({"command":"login","username":"noa","password":"pw"})"));
}

TEST_CASE("parseFindMatchRequest returns false for malformed JSON") {
    REQUIRE_FALSE(parseFindMatchRequest("not valid json"));
}

TEST_CASE("parseFindMatchRequest returns false when command has the wrong JSON type") {
    REQUIRE_FALSE(parseFindMatchRequest(R"({"command":42})"));
}

TEST_CASE("parseFindMatchRequest is case-sensitive about the command name") {
    REQUIRE_FALSE(parseFindMatchRequest(R"({"command":"FindMatch"})"));
}

TEST_CASE("parseFindMatchRequest ignores unrelated extra fields") {
    REQUIRE(parseFindMatchRequest(R"({"command":"findMatch","extra":true})"));
}

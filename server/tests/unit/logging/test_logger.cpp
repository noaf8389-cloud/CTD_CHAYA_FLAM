#include <cstdio>
#include <fstream>
#include <sstream>

#include "../../catch2/catch_amalgamated.hpp"
#include "logging/logger.hpp"

namespace {
    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }
}

TEST_CASE("info/warn/error each write a line tagged with their level and message") {
    const std::string path = "test_logger_levels.log";
    std::remove(path.c_str());
    Logger::init(path);

    Logger::info("hello info");
    Logger::warn("hello warn");
    Logger::error("hello error");

    std::string contents = readFile(path);
    REQUIRE(contents.find("[INFO] hello info") != std::string::npos);
    REQUIRE(contents.find("[WARN] hello warn") != std::string::npos);
    REQUIRE(contents.find("[ERROR] hello error") != std::string::npos);

    std::remove(path.c_str());
}

TEST_CASE("multiple log calls append in order rather than overwriting") {
    const std::string path = "test_logger_order.log";
    std::remove(path.c_str());
    Logger::init(path);

    Logger::info("first");
    Logger::info("second");
    Logger::info("third");

    std::string contents = readFile(path);
    size_t firstPos = contents.find("first");
    size_t secondPos = contents.find("second");
    size_t thirdPos = contents.find("third");
    REQUIRE(firstPos < secondPos);
    REQUIRE(secondPos < thirdPos);

    std::remove(path.c_str());
}

TEST_CASE("re-initializing switches subsequent logs to the new file") {
    const std::string pathA = "test_logger_a.log";
    const std::string pathB = "test_logger_b.log";
    std::remove(pathA.c_str());
    std::remove(pathB.c_str());

    Logger::init(pathA);
    Logger::info("goes to A");

    Logger::init(pathB);
    Logger::info("goes to B");

    std::string contentsA = readFile(pathA);
    std::string contentsB = readFile(pathB);
    REQUIRE(contentsA.find("goes to A") != std::string::npos);
    REQUIRE(contentsA.find("goes to B") == std::string::npos);
    REQUIRE(contentsB.find("goes to B") != std::string::npos);

    std::remove(pathA.c_str());
    std::remove(pathB.c_str());
}

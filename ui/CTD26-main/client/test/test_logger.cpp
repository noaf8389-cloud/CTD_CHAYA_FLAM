#include <cstdio>
#include <fstream>
#include <sstream>

#include "test_framework.hpp"
#include "logging/logger.hpp"

namespace {
    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }
}

TEST(logger_levels_are_tagged_with_the_right_label_and_message) {
    const std::string path = "test_logger_levels.log";
    std::remove(path.c_str());
    Logger::init(path);

    Logger::info("hello info");
    Logger::warn("hello warn");
    Logger::error("hello error");

    std::string contents = readFile(path);
    EXPECT_TRUE(contents.find("[INFO] hello info") != std::string::npos);
    EXPECT_TRUE(contents.find("[WARN] hello warn") != std::string::npos);
    EXPECT_TRUE(contents.find("[ERROR] hello error") != std::string::npos);

    std::remove(path.c_str());
}

TEST(logger_multiple_calls_append_in_order) {
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
    EXPECT_TRUE(firstPos < secondPos);
    EXPECT_TRUE(secondPos < thirdPos);

    std::remove(path.c_str());
}

TEST(logger_reinit_switches_subsequent_logs_to_the_new_file) {
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
    EXPECT_TRUE(contentsA.find("goes to A") != std::string::npos);
    EXPECT_TRUE(contentsA.find("goes to B") == std::string::npos);
    EXPECT_TRUE(contentsB.find("goes to B") != std::string::npos);

    std::remove(pathA.c_str());
    std::remove(pathB.c_str());
}

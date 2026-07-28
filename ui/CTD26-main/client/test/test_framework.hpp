#pragma once
#include <functional>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& all_tests() {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> fn) {
        all_tests().push_back({name, fn});
    }
};

struct AssertionFailure {
    std::string message;
};

#define TEST(name) \
    void name(); \
    static TestRegistrar registrar_##name(#name, name); \
    void name()

#define EXPECT_TRUE(cond) \
    if (!(cond)) throw AssertionFailure{std::string(__FILE__) + ":" + std::to_string(__LINE__) + " EXPECT_TRUE failed: " #cond}

#define EXPECT_EQ(a, b) \
    if (!((a) == (b))) throw AssertionFailure{std::string(__FILE__) + ":" + std::to_string(__LINE__) + " EXPECT_EQ failed: " #a " != " #b}

#define EXPECT_THROWS(expr) \
    do { \
        bool threw = false; \
        try { (expr); } catch (...) { threw = true; } \
        if (!threw) throw AssertionFailure{std::string(__FILE__) + ":" + std::to_string(__LINE__) + " EXPECT_THROWS failed: " #expr " did not throw"}; \
    } while (0)

#define EXPECT_NO_THROW(expr) \
    do { \
        try { (expr); } \
        catch (const std::exception& e) { \
            throw AssertionFailure{std::string(__FILE__) + ":" + std::to_string(__LINE__) + " EXPECT_NO_THROW failed: threw " + e.what()}; \
        } \
    } while (0)

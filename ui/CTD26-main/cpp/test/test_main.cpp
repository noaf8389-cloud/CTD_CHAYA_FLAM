#include "test_framework.hpp"
#include <iostream>

int main() {
    int passed = 0, failed = 0;
    for (const auto& test : all_tests()) {
        try {
            test.fn();
            std::cout << "[PASS] " << test.name << std::endl;
            ++passed;
        } catch (const AssertionFailure& e) {
            std::cout << "[FAIL] " << test.name << " - " << e.message << std::endl;
            ++failed;
        } catch (const std::exception& e) {
            std::cout << "[FAIL] " << test.name << " - unexpected exception: " << e.what() << std::endl;
            ++failed;
        }
    }
    std::cout << passed << " passed, " << failed << " failed" << std::endl;
    return failed == 0 ? 0 : 1;
}

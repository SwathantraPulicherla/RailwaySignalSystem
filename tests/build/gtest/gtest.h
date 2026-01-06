#pragma once
// Minimal Google Test-like framework for testing
#include <iostream>
#include <vector>
#include <functional>
#include <cassert>

class TestRegistry {
private:
    struct TestInfo {
        std::string name;
        std::function<void()> func;
    };
    std::vector<TestInfo> tests_;
    static TestRegistry* instance_;

    TestRegistry() {}

public:
    static TestRegistry& instance() {
        if (!instance_) instance_ = new TestRegistry();
        return *instance_;
    }

    void register_test(const std::string& name, std::function<void()> func) {
        tests_.push_back({name, func});
    }

    int run_all_tests() {
        int failures = 0;
        for (const auto& test : tests_) {
            try {
                test.func();
                std::cout << "[ PASS ] " << test.name << std::endl;
            } catch (const std::exception& e) {
                std::cout << "[ FAIL ] " << test.name << ": " << e.what() << std::endl;
                failures++;
            } catch (...) {
                std::cout << "[ FAIL ] " << test.name << ": Unknown exception" << std::endl;
                failures++;
            }
        }
        return failures;
    }
};

TestRegistry* TestRegistry::instance_ = nullptr;

#define TEST(suite, name) \
    void Test_##suite##_##name(); \
    struct Registrar_##suite##_##name { \
        Registrar_##suite##_##name() { \
            TestRegistry::instance().register_test(#suite "." #name, Test_##suite##_##name); \
        } \
    } registrar_##suite##_##name; \
    void Test_##suite##_##name()

#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_NE(a, b) assert((a) != (b))
#define ASSERT_TRUE(a) assert((a))
#define ASSERT_FALSE(a) assert(!(a))

int RUN_ALL_TESTS() {
    return TestRegistry::instance().run_all_tests();
}

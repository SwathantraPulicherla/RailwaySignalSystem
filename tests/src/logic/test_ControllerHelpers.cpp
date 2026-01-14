/* test_ControllerHelpers.cpp – Auto-generated Expert Google Test Tests */
#include <gtest/gtest.h>
#include <cstdint> // For integer types commonly used for milliseconds
#include "railway/logic/ControllerHelpers.h"

namespace railway::logic {

// Test fixture for ControllerHelpers functions
class ControllerHelpersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No common setup needed for this pure function
    }

    void TearDown() override {
        // No common teardown needed
    }
};

// Test case 1: When lastTickMs is 0, the controller should always be considered fresh.
TEST_F(ControllerHelpersTest, LastTickIsZeroReturnsTrue) {
    railway::Millis lastTickMs = 0;
    railway::Millis now = 1000;
    railway::Millis maxLoopGapMs = 500;

    EXPECT_TRUE(computeControllerFresh(lastTickMs, now, maxLoopGapMs));
}

// Test case 2: The time difference (gap) is less than maxLoopGapMs.
TEST_F(ControllerHelpersTest, GapLessThanMaxReturnsTrue) {
    railway::Millis lastTickMs = 100;
    railway::Millis now = 500;
    railway::Millis maxLoopGapMs = 450; // Gap is 400, 400 < 450

    EXPECT_TRUE(computeControllerFresh(lastTickMs, now, maxLoopGapMs));
}

// Test case 3: The time difference (gap) is exactly equal to maxLoopGapMs.
TEST_F(ControllerHelpersTest, GapEqualsMaxReturnsTrue) {
    railway::Millis lastTickMs = 100;
    railway::Millis now = 550;
    railway::Millis maxLoopGapMs = 450; // Gap is 450, 450 == 450

    EXPECT_TRUE(computeControllerFresh(lastTickMs, now, maxLoopGapMs));
}

// Test case 4: The time difference (gap) is greater than maxLoopGapMs.
TEST_F(ControllerHelpersTest, GapGreaterThanMaxReturnsFalse) {
    railway::Millis lastTickMs = 100;
    railway::Millis now = 600;
    railway::Millis maxLoopGapMs = 450; // Gap is 500, 500 > 450

    EXPECT_FALSE(computeControllerFresh(lastTickMs, now, maxLoopGapMs));
}

// Test case 5: Edge case with large time values, where gap equals max.
TEST_F(ControllerHelpersTest, LargeTimeValuesGapEqualsMax) {
    railway::Millis lastTickMs = 4000000000U; // Example near max uint32_t
    railway::Millis now = 4200000000U;
    railway::Millis maxLoopGapMs = 200000000U; // Gap = 200,000,000

    EXPECT_TRUE(computeControllerFresh(lastTickMs, now, maxLoopGapMs));
}

// Test case 6: Edge case with large time values, where gap exceeds max.
TEST_F(ControllerHelpersTest, LargeTimeValuesGapExceedsMax) {
    railway::Millis lastTickMs = 4000000000U;
    railway::Millis now = 4200000001U; // One millisecond more than the previous test case
    railway::Millis maxLoopGapMs = 200000000U;

    EXPECT_FALSE(computeControllerFresh(lastTickMs, now, maxLoopGapMs));
}

} // namespace railway::logic

/* Skipped due to hardware dependency: */
// None
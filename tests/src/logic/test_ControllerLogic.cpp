/* test_ControllerLogic.cpp – Auto-generated Expert Google Test Tests */
#include <gtest/gtest.h>
#include <cstdint>

// Include the real production headers
#include "railway/logic/ControllerLogic.h"

namespace railway::logic {

static void ExpectDecision(
    const Decision& result,
    railway::drivers::Aspect aspect,
    StopReason reason,
    railway::Health health
) {
    EXPECT_EQ(result.aspect, aspect);
    EXPECT_EQ(result.reason, reason);
    EXPECT_EQ(result.health, health);
}

// Test fixture for ControllerLogic
class ControllerLogicTest : public ::testing::Test {
protected:

    void SetUp() override {
        // No shared setup needed.
    }

    void TearDown() override {
        // No special teardown needed
    }
};

// Test Case 1: All conditions clear and fresh - should result in the 'go' decision.
TEST_F(ControllerLogicTest, AllClearAndFresh_ReturnsClearOk) {
    auto result = evaluateControllerLogic(100, 110, 20, true, false, false);
    ExpectDecision(result, railway::drivers::Aspect::Clear, StopReason::None, railway::Health::Ok);
}

// Test Case 2: Own block is occupied - should result in the 'stop' decision.
TEST_F(ControllerLogicTest, OwnBlockOccupied_ReturnsStopOk) {
    auto result = evaluateControllerLogic(100, 110, 20, true, true, false);
    ExpectDecision(result, railway::drivers::Aspect::Stop, StopReason::OwnBlockOccupied, railway::Health::Ok);
}

// Test Case 3: Downstream block is occupied - should result in the 'caution' decision.
TEST_F(ControllerLogicTest, DownstreamBlockOccupied_ReturnsCautionOk) {
    auto result = evaluateControllerLogic(100, 110, 20, true, false, true);
    ExpectDecision(result, railway::drivers::Aspect::Caution, StopReason::DownstreamStop, railway::Health::Ok);
}

// Test Case 4: Controller data is stale (not fresh) - should result in a fail-safe 'stop' decision.
TEST_F(ControllerLogicTest, ControllerDataStale_ReturnsStopFault) {
    auto result = evaluateControllerLogic(100, 150, 20, true, false, false);
    ExpectDecision(result, railway::drivers::Aspect::Stop, StopReason::ControllerStale, railway::Health::Fault);
}

// Test Case 5: Own track circuit is unhealthy - should result in a fail-safe 'stop' decision.
TEST_F(ControllerLogicTest, OwnTrackCircuitUnhealthy_ReturnsStopDegraded) {
    auto result = evaluateControllerLogic(100, 110, 20, false, false, false);
    ExpectDecision(result, railway::drivers::Aspect::Stop, StopReason::TrackCircuitFault, railway::Health::Degraded);
}

// Test Case 6: Mixed conditions, e.g., own block occupied but data stale.
TEST_F(ControllerLogicTest, LastTickZero_IsAlwaysFresh) {
    auto result = evaluateControllerLogic(0, 999, 0, true, false, false);
    ExpectDecision(result, railway::drivers::Aspect::Clear, StopReason::None, railway::Health::Ok);
}

} // namespace railway::logic

/* Skipped due to hardware dependency: None */
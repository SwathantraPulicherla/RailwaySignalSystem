/* AI-TESTGEN-SECTION
Section: base_10cd5cc0
Source: src/logic/Interlocking.cpp
Source-SHA256: 953f3bf73e1487b173132ca9a87f8ad4b7913ebc2227e2c6a2d882fd58c172d4
Section-SHA256: f26112ff15bbee0ba01800cc7d643ee54bbe951a260c6fdcccaff3771b00b26c
Kind: base
Approved: false
Reviewed-By: 
Reviewed-At: 
*/
#include <gtest/gtest.h>
#include <cstdint>
#include "railway/logic/Interlocking.h"

namespace ai_testgen_section_10cd5cc0 {

/* test_Interlocking.cpp – Auto-generated Expert Google Test Tests */

// Include the real production header

// Define a test fixture
class InterlockingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No specific setup needed for this pure function
    }

    void TearDown() override {
        // No specific teardown needed
    }
};

// Test case 1: Controller is stale, should always result in Stop
TEST_F(InterlockingTest, ControllerStaleReturnsStop) {
    railway::logic::Inputs in{};
    in.controllerFresh = false; // Make controller stale
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = false;

    railway::logic::Decision out = railway::logic::evaluate(in);

    EXPECT_EQ(out.aspect, railway::drivers::Aspect::Stop);
    EXPECT_EQ(out.reason, railway::logic::StopReason::ControllerStale);
    EXPECT_EQ(out.health, railway::Health::Fault);
}

// Test case 2: Own track circuit is unhealthy, should result in Stop
TEST_F(InterlockingTest, OwnTrackCircuitUnhealthyReturnsStop) {
    railway::logic::Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = false; // Make own track circuit unhealthy
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = false;

    railway::logic::Decision out = railway::logic::evaluate(in);

    EXPECT_EQ(out.aspect, railway::drivers::Aspect::Stop);
    EXPECT_EQ(out.reason, railway::logic::StopReason::TrackCircuitFault);
    EXPECT_EQ(out.health, railway::Health::Degraded);
}

// Test case 3: Own block is occupied, should result in Stop
TEST_F(InterlockingTest, OwnBlockOccupiedReturnsStop) {
    railway::logic::Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = true; // Make own block occupied
    in.downstreamBlockOccupied = false;

    railway::logic::Decision out = railway::logic::evaluate(in);

    EXPECT_EQ(out.aspect, railway::drivers::Aspect::Stop);
    EXPECT_EQ(out.reason, railway::logic::StopReason::OwnBlockOccupied);
    EXPECT_EQ(out.health, railway::Health::Ok);
}

// Test case 4: Downstream block is occupied, should result in Caution
TEST_F(InterlockingTest, DownstreamBlockOccupiedReturnsCaution) {
    railway::logic::Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = true; // Make downstream block occupied

    railway::logic::Decision out = railway::logic::evaluate(in);

    EXPECT_EQ(out.aspect, railway::drivers::Aspect::Caution);
    EXPECT_EQ(out.reason, railway::logic::StopReason::DownstreamStop);
    EXPECT_EQ(out.health, railway::Health::Ok);
}

// Test case 5: All conditions clear, should result in Clear
TEST_F(InterlockingTest, AllClearReturnsClear) {
    railway::logic::Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = false; // Both blocks are clear

    railway::logic::Decision out = railway::logic::evaluate(in);

    EXPECT_EQ(out.aspect, railway::drivers::Aspect::Clear);
    EXPECT_EQ(out.reason, railway::logic::StopReason::None);
    EXPECT_EQ(out.health, railway::Health::Ok);
}

// Skipped due to hardware dependency: None

}  // namespace ai_testgen_section_10cd5cc0

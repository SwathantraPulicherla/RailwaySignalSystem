/* test_Interlocking.cpp – Auto-generated Expert Google Test Tests */
#include <gtest/gtest.h>
#include <stdint.h>
#include "railway/logic/Interlocking.h"

// Bring types into global namespace for convenience in tests
using namespace railway::logic;
using namespace railway::drivers;
using namespace railway;

struct InterlockingTest : public ::testing::Test {
    void SetUp() override {
        // No shared state to set up for this pure function
    }

    void TearDown() override {
        // No shared state to tear down
    }
};

TEST_F(InterlockingTest, ControllerStaleReturnsFaultStop) {
    Inputs in{};
    in.controllerFresh = false; // Trigger this condition
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = false;

    Decision out = evaluate(in);

    EXPECT_EQ(out.aspect, Aspect::Stop);
    EXPECT_EQ(out.reason, StopReason::ControllerStale);
    EXPECT_EQ(out.health, Health::Fault);
}

TEST_F(InterlockingTest, OwnTrackCircuitFaultReturnsDegradedStop) {
    Inputs in{};
    in.controllerFresh = true; // Pass previous check
    in.ownTrackCircuitHealthy = false; // Trigger this condition
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = false;

    Decision out = evaluate(in);

    EXPECT_EQ(out.aspect, Aspect::Stop);
    EXPECT_EQ(out.reason, StopReason::TrackCircuitFault);
    EXPECT_EQ(out.health, Health::Degraded);
}

TEST_F(InterlockingTest, OwnBlockOccupiedReturnsOkStop) {
    Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = true; // Pass previous checks
    in.ownBlockOccupied = true; // Trigger this condition
    in.downstreamBlockOccupied = false;

    Decision out = evaluate(in);

    EXPECT_EQ(out.aspect, Aspect::Stop);
    EXPECT_EQ(out.reason, StopReason::OwnBlockOccupied);
    EXPECT_EQ(out.health, Health::Ok);
}

TEST_F(InterlockingTest, DownstreamBlockOccupiedReturnsOkCaution) {
    Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = false; // Pass previous checks
    in.downstreamBlockOccupied = true; // Trigger this condition

    Decision out = evaluate(in);

    EXPECT_EQ(out.aspect, Aspect::Caution);
    EXPECT_EQ(out.reason, StopReason::DownstreamStop);
    EXPECT_EQ(out.health, Health::Ok);
}

TEST_F(InterlockingTest, AllClearReturnsOkClear) {
    Inputs in{};
    in.controllerFresh = true;
    in.ownTrackCircuitHealthy = true;
    in.ownBlockOccupied = false;
    in.downstreamBlockOccupied = false; // Pass all conditions above

    Decision out = evaluate(in);

    EXPECT_EQ(out.aspect, Aspect::Clear);
    EXPECT_EQ(out.reason, StopReason::None);
    EXPECT_EQ(out.health, Health::Ok);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Skipped due to hardware dependency: None
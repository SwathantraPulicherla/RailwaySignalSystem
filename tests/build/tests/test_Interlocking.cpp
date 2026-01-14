/* test_Interlocking.cpp – Auto-generated Expert Google Test Tests */
#include <gtest/gtest.h>
#include <cstdint>
#include "railway/logic/Interlocking.h"

// Helper to make input creation more concise
namespace railway::logic {
struct TestInputs : public Inputs {
    TestInputs() {
        controllerFresh = true;
        ownTrackCircuitHealthy = true;
        ownBlockOccupied = false;
        downstreamBlockOccupied = false;
    }
};
} // namespace railway::logic


class InterlockingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Any common setup for tests can go here.
        // For this stateless function, it's not strictly necessary.
    }

    void TearDown() override {
        // Any common cleanup.
    }
};

TEST_F(InterlockingTest, Evaluate_ControllerStale_ReturnsStopFault) {
    railway::logic::TestInputs in;
    in.controllerFresh = false;

    railway::logic::Decision result = railway::logic::evaluate(in);

    EXPECT_EQ(result.aspect, railway::drivers::Aspect::Stop);
    EXPECT_EQ(result.reason, railway::logic::StopReason::ControllerStale);
    EXPECT_EQ(result.health, railway::Health::Fault);
}

TEST_F(InterlockingTest, Evaluate_OwnTrackCircuitUnhealthy_ReturnsStopDegraded) {
    railway::logic::TestInputs in;
    in.ownTrackCircuitHealthy = false;

    railway::logic::Decision result = railway::logic::evaluate(in);

    EXPECT_EQ(result.aspect, railway::drivers::Aspect::Stop);
    EXPECT_EQ(result.reason, railway::logic::StopReason::TrackCircuitFault);
    EXPECT_EQ(result.health, railway::Health::Degraded);
}

TEST_F(InterlockingTest, Evaluate_OwnBlockOccupied_ReturnsStopOk) {
    railway::logic::TestInputs in;
    in.ownBlockOccupied = true;

    railway::logic::Decision result = railway::logic::evaluate(in);

    EXPECT_EQ(result.aspect, railway::drivers::Aspect::Stop);
    EXPECT_EQ(result.reason, railway::logic::StopReason::OwnBlockOccupied);
    EXPECT_EQ(result.health, railway::Health::Ok);
}

TEST_F(InterlockingTest, Evaluate_DownstreamBlockOccupied_ReturnsCautionOk) {
    railway::logic::TestInputs in;
    in.downstreamBlockOccupied = true;

    railway::logic::Decision result = railway::logic::evaluate(in);

    EXPECT_EQ(result.aspect, railway::drivers::Aspect::Caution);
    EXPECT_EQ(result.reason, railway::logic::StopReason::DownstreamStop);
    EXPECT_EQ(result.health, railway::Health::Ok);
}

TEST_F(InterlockingTest, Evaluate_AllConditionsClear_ReturnsClearOk) {
    railway::logic::TestInputs in;
    // All default values are already set for a clear state in TestInputs constructor

    railway::logic::Decision result = railway::logic::evaluate(in);

    EXPECT_EQ(result.aspect, railway::drivers::Aspect::Clear);
    EXPECT_EQ(result.reason, railway::logic::StopReason::None);
    EXPECT_EQ(result.health, railway::Health::Ok);
}

// Skipped due to hardware dependency: None
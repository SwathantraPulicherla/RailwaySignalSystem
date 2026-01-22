/* AI-TEST-SECTION
Section: BASE_TESTS
Source: src/app/BlockController.cpp
Status: Approved
Approved: true
Reviewed-By: Mia
Reviewed-At: 2026-01-16T05:30:00.000000+00:00
*/
#include <gtest/gtest.h>
#include <cstdint>
#include <map>
#include <tuple>
#include <vector>
#include "railway/app/BlockController.h"
#include "railway/drivers/TrackCircuitInput.h"
#include "railway/drivers/SignalHead.h"
#include "railway/hal/IGpio.h"
#include "railway/hal/IClock.h"

namespace ai_test_section_base {

/* test_BlockController.cpp – Auto-generated Expert Google Test Tests */


namespace test_support {

class FakeGpio final : public ::railway::hal::IGpio {
public:
    struct PinState {
        ::railway::hal::PinMode mode{::railway::hal::PinMode::Input};
        ::railway::hal::PinLevel level{::railway::hal::PinLevel::Low};
    };

    std::map<::railway::hal::Pin, PinState> pins;
    std::vector<std::tuple<::railway::hal::Pin, ::railway::hal::PinMode>> configureCalls;
    std::vector<std::tuple<::railway::hal::Pin, ::railway::hal::PinLevel>> writeCalls;

    void configure(::railway::hal::Pin pin, ::railway::hal::PinMode mode) override {
        pins[pin].mode = mode;
        configureCalls.emplace_back(pin, mode);
    }

    ::railway::hal::PinLevel read(::railway::hal::Pin pin) const override {
        auto it = pins.find(pin);
        if (it == pins.end()) {
            return ::railway::hal::PinLevel::Low;
        }
        return it->second.level;
    }

    void write(::railway::hal::Pin pin, ::railway::hal::PinLevel level) override {
        pins[pin].level = level;
        writeCalls.emplace_back(pin, level);
    }

    void setInputLevel(::railway::hal::Pin pin, ::railway::hal::PinLevel level) {
        pins[pin].level = level;
    }
};

class TestClock final : public ::railway::hal::IClock {
public:
    ::railway::Millis now_{0};

    ::railway::Millis nowMs() const override {
        return now_;
    }

    void setNow(::railway::Millis ms) {
        now_ = ms;
    }
};

} // namespace test_support

class AISEC_BASE_BlockControllerTest : public ::testing::Test {
protected:
    static constexpr ::railway::hal::Pin OWN_TC_PIN = 10;
    static constexpr ::railway::hal::Pin DOWNSTREAM_TC_PIN = 11;

    static constexpr ::railway::hal::Pin RED_PIN = 21;
    static constexpr ::railway::hal::Pin YELLOW_PIN = 22;
    static constexpr ::railway::hal::Pin GREEN_PIN = 23;

    test_support::FakeGpio gpio_;
    test_support::TestClock clock_;

    ::railway::drivers::TrackCircuitInput::Config own_cfg_{};
    ::railway::drivers::TrackCircuitInput::Config downstream_cfg_{};
    ::railway::drivers::SignalHead::Config signal_cfg_{};
    ::railway::app::BlockController::Config controller_cfg_{};

    std::unique_ptr<::railway::drivers::TrackCircuitInput> own_track_;
    std::unique_ptr<::railway::drivers::TrackCircuitInput> downstream_track_;
    std::unique_ptr<::railway::drivers::SignalHead> signal_;
    std::unique_ptr<::railway::app::BlockController> controller_;

    void buildController() {
        own_track_ = std::make_unique<::railway::drivers::TrackCircuitInput>(own_cfg_, gpio_);
        downstream_track_ = std::make_unique<::railway::drivers::TrackCircuitInput>(downstream_cfg_, gpio_);
        signal_ = std::make_unique<::railway::drivers::SignalHead>(signal_cfg_, gpio_);
        controller_ = std::make_unique<::railway::app::BlockController>(
            controller_cfg_, clock_, *own_track_, *downstream_track_, *signal_);
    }

    void SetUp() override {
        own_cfg_.pin = OWN_TC_PIN;
        own_cfg_.activeLow = true;
        own_cfg_.debounceMs = 0;
        own_cfg_.stuckLowFaultMs = 3000;

        downstream_cfg_.pin = DOWNSTREAM_TC_PIN;
        downstream_cfg_.activeLow = true;
        downstream_cfg_.debounceMs = 0;
        downstream_cfg_.stuckLowFaultMs = 3000;

        signal_cfg_.redPin = RED_PIN;
        signal_cfg_.yellowPin = YELLOW_PIN;
        signal_cfg_.greenPin = GREEN_PIN;
        signal_cfg_.activeHigh = true;

        controller_cfg_.maxLoopGapMs = 100;

        // Default input state: clear/energized for active-low track circuit implies GPIO reads High.
        gpio_.setInputLevel(OWN_TC_PIN, ::railway::hal::PinLevel::High);
        gpio_.setInputLevel(DOWNSTREAM_TC_PIN, ::railway::hal::PinLevel::High);

        clock_.setNow(0);
        buildController();
        controller_->init();
    }
};

TEST_F(AISEC_BASE_BlockControllerTest, AISEC_BASE_Init_ConfiguresDriversAndSetsSignalToDecisionAspect) {
    // init() configures both track circuits and the signal head.
    // TrackCircuitInput::init configures InputPullup, SignalHead::init configures three outputs.
    ASSERT_GE(gpio_.configureCalls.size(), 5u);

    // BlockController::init sets last_ = evaluate(Inputs{}), and sets the signal to last_.aspect.
    // For Inputs{} defaults in Interlocking.h, controllerFresh is false, so the decision is Stop.
    EXPECT_EQ(controller_->lastDecision().aspect, ::railway::drivers::Aspect::Stop);
    EXPECT_EQ(signal_->currentAspect(), ::railway::drivers::Aspect::Stop);
}

TEST_F(AISEC_BASE_BlockControllerTest, AISEC_BASE_Tick_AllClear_UpdatesDecisionToClear) {
    // lastTickMs_ was set to 0 at init (clock=0), and computeControllerFresh treats 0 as fresh.
    clock_.setNow(10);
    controller_->tick();

    EXPECT_EQ(controller_->lastDecision().aspect, ::railway::drivers::Aspect::Clear);
    EXPECT_EQ(signal_->currentAspect(), ::railway::drivers::Aspect::Clear);
}

TEST_F(AISEC_BASE_BlockControllerTest, AISEC_BASE_Tick_OwnOccupied_FailsSafeToStop) {
    // activeLow track circuit: GPIO Low => not clear => occupied.
    gpio_.setInputLevel(OWN_TC_PIN, ::railway::hal::PinLevel::Low);
    clock_.setNow(10);
    controller_->tick();

    EXPECT_EQ(controller_->lastDecision().aspect, ::railway::drivers::Aspect::Stop);
    EXPECT_EQ(signal_->currentAspect(), ::railway::drivers::Aspect::Stop);
    EXPECT_EQ(controller_->lastDecision().reason, ::railway::logic::StopReason::OwnBlockOccupied);
}

TEST_F(AISEC_BASE_BlockControllerTest, AISEC_BASE_Tick_DownstreamOccupied_ProducesCaution) {
    gpio_.setInputLevel(DOWNSTREAM_TC_PIN, ::railway::hal::PinLevel::Low);
    clock_.setNow(10);
    controller_->tick();

    EXPECT_EQ(controller_->lastDecision().aspect, ::railway::drivers::Aspect::Caution);
    EXPECT_EQ(signal_->currentAspect(), ::railway::drivers::Aspect::Caution);
    EXPECT_EQ(controller_->lastDecision().reason, ::railway::logic::StopReason::DownstreamStop);
}

TEST_F(AISEC_BASE_BlockControllerTest, AISEC_BASE_Tick_StuckLowBeyondThreshold_MarksTrackFault) {
    // Keep the controller "fresh" while the track-circuit stuck-low timer elapses.
    controller_cfg_.maxLoopGapMs = own_cfg_.stuckLowFaultMs + 100;
    clock_.setNow(0);
    buildController();
    controller_->init();

    // Keep own track "not clear" (occupied) long enough for TrackCircuitInput to mark unhealthy.
    gpio_.setInputLevel(OWN_TC_PIN, ::railway::hal::PinLevel::Low);

    // First tick starts the stuck-low timer.
    clock_.setNow(10);
    controller_->tick();

    // Second tick exceeds stuckLowFaultMs, making isHealthy() false.
    clock_.setNow(10 + own_cfg_.stuckLowFaultMs + 1);
    controller_->tick();

    EXPECT_EQ(controller_->lastDecision().aspect, ::railway::drivers::Aspect::Stop);
    EXPECT_EQ(controller_->lastDecision().reason, ::railway::logic::StopReason::TrackCircuitFault);
    EXPECT_EQ(controller_->lastDecision().health, ::railway::Health::Degraded);
    EXPECT_EQ(signal_->currentAspect(), ::railway::drivers::Aspect::Stop);
}

TEST_F(AISEC_BASE_BlockControllerTest, AISEC_BASE_Tick_ControllerStale_TakesPriority) {
    // Rebuild with a non-zero init time so computeControllerFresh can become false.
    clock_.setNow(1);
    buildController();
    controller_->init();

    // Ensure inputs are otherwise "good".
    gpio_.setInputLevel(OWN_TC_PIN, ::railway::hal::PinLevel::High);
    gpio_.setInputLevel(DOWNSTREAM_TC_PIN, ::railway::hal::PinLevel::High);

    // Exceed maxLoopGapMs relative to lastTickMs_ (which is 1).
    clock_.setNow(1 + controller_cfg_.maxLoopGapMs + 1);
    controller_->tick();

    EXPECT_EQ(controller_->lastDecision().aspect, ::railway::drivers::Aspect::Stop);
    EXPECT_EQ(controller_->lastDecision().reason, ::railway::logic::StopReason::ControllerStale);
    EXPECT_EQ(controller_->lastDecision().health, ::railway::Health::Fault);
}

/* Not directly included (hardware-touching)
None
*/

}  // namespace

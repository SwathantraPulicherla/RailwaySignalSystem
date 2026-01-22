/* AI-TEST-SECTION
Section: BASE_TESTS
Source: src/drivers/TrackCircuitInput.cpp
Status: Approved
Approved: true
Reviewed-By: Mia
Reviewed-At: 2026-01-16T12:05:35.683151+00:00
*/
#include <gtest/gtest.h>
#include <cstdint> // For uint8_t, uint32_t
#include <memory>
#include "Arduino_stubs.h" // For Millis
#include "railway/drivers/TrackCircuitInput.h"
#include "railway/hal/IGpio.h"

namespace ai_test_section_base {

/* test_TrackCircuitInput.cpp – Auto-generated Expert Google Test Tests */

// Define a FakeGpio class that implements the railway::hal::IGpio interface.
// This allows controlling its behavior deterministically for testing.
class FakeGpio : public railway::hal::IGpio {
public:
    railway::hal::PinMode configuredMode_ = railway::hal::PinMode::Input;
    railway::hal::Pin configuredPin_ = 0;
    railway::hal::PinLevel nextReadLevel_ = railway::hal::PinLevel::Low;
    mutable railway::hal::Pin lastReadPin_ = 0;
    railway::hal::Pin lastWritePin_ = 0;
    railway::hal::PinLevel lastWriteLevel_ = railway::hal::PinLevel::Low;

    void configure(railway::hal::Pin pin, railway::hal::PinMode mode) override {
        configuredPin_ = pin;
        configuredMode_ = mode;
    }
    railway::hal::PinLevel read(railway::hal::Pin pin) const override {
        lastReadPin_ = pin;
        return nextReadLevel_;
    }
    void write(railway::hal::Pin pin, railway::hal::PinLevel level) override {
        lastWritePin_ = pin;
        lastWriteLevel_ = level;
    }

    void reset() {
        configuredMode_ = railway::hal::PinMode::Input;
        configuredPin_ = 0;
        nextReadLevel_ = railway::hal::PinLevel::Low;
        lastReadPin_ = 0;
        lastWritePin_ = 0;
        lastWriteLevel_ = railway::hal::PinLevel::Low;
    }
};

using ::railway::drivers::TrackCircuitInput;

class AISEC_BASE_TrackCircuitInputTest : public ::testing::Test {
protected:
    TrackCircuitInput::Config cfg_ = {
        .pin = 5,
        .activeLow = true, // Default for most tests
        .debounceMs = 100,
        .stuckLowFaultMs = 500,
    };

    FakeGpio gpio_;
    std::unique_ptr<TrackCircuitInput> circuit_;

    void Reinit(const TrackCircuitInput::Config& cfg, railway::hal::PinLevel initialReadLevel) {
        cfg_ = cfg;
        gpio_.reset();
        gpio_.nextReadLevel_ = initialReadLevel;
        circuit_ = std::make_unique<TrackCircuitInput>(cfg_, gpio_);
        circuit_->init();
    }

    void SetUp() override {
        // Default initial GPIO state for TrackCircuitInput::init().
        // With activeLow=true, PinLevel::High results in rawClear=true (clear/unoccupied).
        Reinit(cfg_, railway::hal::PinLevel::High);
    }

    // Helper to advance time and call update
    void AdvanceTimeAndPump(railway::Millis& nowMs, railway::Millis advanceBy) {
        nowMs += advanceBy;
        circuit_->update(nowMs);
    }
};

// Test cases for TrackCircuitInput::init()
TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_InitConfiguresGpioCorrectly) {
    EXPECT_EQ(gpio_.configuredPin_, cfg_.pin);
    EXPECT_EQ(gpio_.configuredMode_, railway::hal::PinMode::InputPullup);
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_InitSetsInitialStateAsClearWhenActiveLowAndPinHigh) {
    // Default config: activeLow = true, gpio_.nextReadLevel_ = PinLevel::High
    // This should result in isOccupied() == false
    EXPECT_TRUE(circuit_->isHealthy());
    EXPECT_FALSE(circuit_->isOccupied());
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_InitSetsInitialStateAsOccupiedWhenActiveLowAndPinLow) {
    // Re-init with initial LOW for init().
    auto cfg = cfg_;
    Reinit(cfg, railway::hal::PinLevel::Low);
    // activeLow = true, PinLevel::Low -> readRawClear() is false.
    EXPECT_TRUE(circuit_->isHealthy()); // Always starts healthy
    EXPECT_TRUE(circuit_->isOccupied());
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_InitSetsInitialStateAsClearWhenActiveHighAndPinLow) {
    // Re-init with activeLow = false for active-high logic.
    auto cfg = cfg_;
    cfg.activeLow = false;
    // activeLow = false, PinLevel::Low -> readRawClear() is true (clear/unoccupied).
    Reinit(cfg, railway::hal::PinLevel::Low);
    EXPECT_TRUE(circuit_->isHealthy());
    EXPECT_FALSE(circuit_->isOccupied());
}

// Test cases for TrackCircuitInput::update()

// Normal operation: Clear -> Occupied -> Clear with debounce
TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateDebouncesStateChangeFromClearToOccupied) {
    railway::Millis nowMs = 0; // Simulate time
    EXPECT_FALSE(circuit_->isOccupied()); // Initial state from SetUp (Clear)

    // Pin goes Low, indicating "Not Clear" / Occupied
    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low;
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 10
    EXPECT_FALSE(circuit_->isOccupied()); // Still debouncing

    // Advance time just before debounceMs threshold
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs - 1); // nowMs = 10 + 99 = 109
    EXPECT_FALSE(circuit_->isOccupied()); // Still debouncing

    // Advance time past debounceMs threshold
    AdvanceTimeAndPump(nowMs, 1); // nowMs = 110 (10 + cfg_.debounceMs)
    EXPECT_TRUE(circuit_->isOccupied()); // Debounced, now occupied
    EXPECT_TRUE(circuit_->isHealthy()); // Should still be healthy
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateDebouncesStateChangeFromOccupiedToClear) {
    railway::Millis nowMs = 0;

    // First, make the circuit occupied
    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low;
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 10
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs + 1); // nowMs = 111. Stable occupied.
    EXPECT_TRUE(circuit_->isOccupied());

    // Circuit becomes clear again (Pin High)
    gpio_.nextReadLevel_ = railway::hal::PinLevel::High;
    AdvanceTimeAndPump(nowMs, 20); // nowMs = 131
    EXPECT_TRUE(circuit_->isOccupied()); // Still debouncing as clear

    // Advance time just before debounceMs threshold
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs - 1); // nowMs = 131 + 99 = 230
    EXPECT_TRUE(circuit_->isOccupied()); // Still debouncing

    // Advance time past debounceMs threshold
    AdvanceTimeAndPump(nowMs, 1); // nowMs = 231
    EXPECT_FALSE(circuit_->isOccupied()); // Debounced, now Clear
    EXPECT_TRUE(circuit_->isHealthy()); // Should be healthy
}

// Fault detection: stuckLowFaultMs
TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateSetsHealthyToFalseWhenStuckOccupiedBeyondThreshold) {
    railway::Millis nowMs = 0;
    EXPECT_FALSE(circuit_->isOccupied()); // Initial Clear state
    EXPECT_TRUE(circuit_->isHealthy());

    // Circuit becomes occupied and stays occupied
    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low;
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 10
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs + 1); // nowMs = 111. Stable occupied.
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_TRUE(circuit_->isHealthy()); // Still healthy, stuckLowSinceMs_ just started

    // Advance time just before stuckLowFaultMs threshold
    AdvanceTimeAndPump(nowMs, cfg_.stuckLowFaultMs - 1); // nowMs = 111 + 499 = 610
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_TRUE(circuit_->isHealthy()); // Still healthy

    // Advance time past stuckLowFaultMs threshold
    AdvanceTimeAndPump(nowMs, 1); // nowMs = 611
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_FALSE(circuit_->isHealthy()); // Now unhealthy
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateResetsStuckLowFaultWhenCircuitBecomesClear) {
    railway::Millis nowMs = 0;

    // Force circuit into an unhealthy state first
    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low; // Occupied
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 10
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs + 1); // nowMs = 111. Stable occupied.
    AdvanceTimeAndPump(nowMs, cfg_.stuckLowFaultMs + 1); // nowMs = 612. Now unhealthy.
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_FALSE(circuit_->isHealthy());

    // Circuit becomes clear (Pin High)
    gpio_.nextReadLevel_ = railway::hal::PinLevel::High;
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 622
    EXPECT_TRUE(circuit_->isOccupied()); // Still debouncing as clear
    EXPECT_FALSE(circuit_->isHealthy()); // Still unhealthy

    // Advance time past debounceMs threshold for clear state
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs + 1); // nowMs = 622 + 101 = 723
    EXPECT_FALSE(circuit_->isOccupied()); // Now Clear
    EXPECT_TRUE(circuit_->isHealthy()); // Should be healthy again
}

// Edge cases for debounceMs and stuckLowFaultMs
TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateWithZeroDebounceMsChangesStateImmediately) {
    auto cfg = cfg_;
    cfg.debounceMs = 0;
    Reinit(cfg, railway::hal::PinLevel::High);
    railway::Millis nowMs = 0;
    EXPECT_FALSE(circuit_->isOccupied()); // Initially Clear

    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low; // Occupied
    AdvanceTimeAndPump(nowMs, 1); // nowMs = 1
    EXPECT_TRUE(circuit_->isOccupied()); // Should change immediately
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateWithZeroStuckLowFaultMsGoesUnhealthyImmediatelyIfOccupied) {
    auto cfg = cfg_;
    cfg.stuckLowFaultMs = 0;
    Reinit(cfg, railway::hal::PinLevel::High);
    railway::Millis nowMs = 0;
    EXPECT_FALSE(circuit_->isOccupied()); // Initially Clear
    EXPECT_TRUE(circuit_->isHealthy());

    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low; // Occupied
    AdvanceTimeAndPump(nowMs, 1); // nowMs = 1 (becomes raw occupied)
    AdvanceTimeAndPump(nowMs, cfg_.debounceMs + 1); // nowMs = 1 + 101 = 102. Stable occupied.
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_FALSE(circuit_->isHealthy()); // Should be immediately unhealthy
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateHandlesInitialOccupiedStateWithZeroStuckLowFaultMs) {
    auto cfg = cfg_;
    cfg.stuckLowFaultMs = 0;
    cfg.activeLow = false; // For active-high wiring, PinLevel::High => not clear.
    Reinit(cfg, railway::hal::PinLevel::High);
    // At init: activeLow is false, PinLevel::High -> readRawClear() is false -> occupied.
    EXPECT_TRUE(circuit_->isOccupied());
    // init() sets healthy=true; the fault is detected during update().
    EXPECT_TRUE(circuit_->isHealthy());

    railway::Millis nowMs = 0;
    circuit_->update(nowMs);
    EXPECT_FALSE(circuit_->isHealthy());
}

// Time wrap-around tests (using uint32_t arithmetic for Millis)
TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateHandlesTimeWrapAroundForDebounce) {
    auto cfg = cfg_;
    cfg.debounceMs = 100;
    Reinit(cfg, railway::hal::PinLevel::High);
    railway::Millis nowMs = 0xFFFFFFFF - 50; // Close to wrap around
    EXPECT_FALSE(circuit_->isOccupied()); // Initial state is Clear

    // Circuit becomes occupied (Pin Low)
    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low;
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 0xFFFFFFFF - 40. rawClear_ changes.
    EXPECT_FALSE(circuit_->isOccupied()); // Still debouncing

    // Advance by the full debounce interval; unsigned Millis arithmetic naturally handles wrap-around.
    AdvanceTimeAndPump(nowMs, cfg.debounceMs); // nowMs wraps; delta is exactly debounceMs
    EXPECT_TRUE(circuit_->isOccupied()); // Debounced.
}

TEST_F(AISEC_BASE_TrackCircuitInputTest, AISEC_BASE_UpdateHandlesTimeWrapAroundForStuckLowFault) {
    auto cfg = cfg_;
    cfg.stuckLowFaultMs = 500;
    Reinit(cfg, railway::hal::PinLevel::High);
    railway::Millis nowMs = 0xFFFFFFFF - 50; // Close to wrap around

    // First, make it stable occupied
    gpio_.nextReadLevel_ = railway::hal::PinLevel::Low;
    AdvanceTimeAndPump(nowMs, 10); // nowMs = 0xFFFFFFFF - 40
    AdvanceTimeAndPump(nowMs, cfg.debounceMs + 1); // nowMs wraps
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_TRUE(circuit_->isHealthy()); // Still healthy, stuckLowSinceMs_ is now set.

    // Advance just before fault threshold, potentially crossing wrap
    AdvanceTimeAndPump(nowMs, cfg.stuckLowFaultMs - 1);
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_TRUE(circuit_->isHealthy());

    // Advance past fault threshold
    AdvanceTimeAndPump(nowMs, 1); // nowMs = 560
    EXPECT_TRUE(circuit_->isOccupied());
    EXPECT_FALSE(circuit_->isHealthy()); // Unhealthy after wrap
}

/*
Not directly included (hardware-touching) dependencies:
- railway::hal::IGpio (stubbed interface)
*/

}  // namespace

/* AI-TEST-SECTION
Section: MCDC_TESTS
Source: src/drivers/TrackCircuitInput.cpp
Status: Approved
Approved: true
Reviewed-By: Mia
Reviewed-At: 2026-01-16T19:13:24.621111+00:00
*/
#include <gtest/gtest.h>
#include <cstdint>
#include <memory> // For std::unique_ptr
#include "railway/drivers/TrackCircuitInput.h"
#include "railway/hal/IGpio.h" // For the interface definition
#include "Arduino_stubs.h"     // For railway::Millis (uint32_t)
#include <memory>

namespace ai_test_section_mcdc_1 {

/* test_TrackCircuitInput.cpp – Auto-generated Expert Google Test Tests */



// Fake implementation for ::railway::hal::IGpio
namespace test_doubles {
class FakeGpio : public ::railway::hal::IGpio {
public:
    ::railway::hal::Pin configured_pin_ = static_cast<::railway::hal::Pin>(-1);
    ::railway::hal::PinMode configured_mode_ = ::railway::hal::PinMode::Input;
    ::railway::hal::PinLevel read_level_ = ::railway::hal::PinLevel::Low; // Default level for read()
    int configure_calls_ = 0;
    mutable int read_calls_ = 0;
    int write_calls_ = 0;
    ::railway::hal::Pin last_write_pin_ = static_cast<::railway::hal::Pin>(0);
    ::railway::hal::PinLevel last_write_level_ = ::railway::hal::PinLevel::Low;

    void configure(::railway::hal::Pin pin, ::railway::hal::PinMode mode) override {
        configured_pin_ = pin;
        configured_mode_ = mode;
        configure_calls_++;
    }

    ::railway::hal::PinLevel read(::railway::hal::Pin /*pin*/) const override {
        read_calls_++;
        return read_level_;
    }

    void write(::railway::hal::Pin pin, ::railway::hal::PinLevel level) override {
        last_write_pin_ = pin;
        last_write_level_ = level;
        write_calls_++;
    }

    // Helper to control the return value of read()
    void set_read_level(::railway::hal::PinLevel level) {
        read_level_ = level;
    }
};
} // namespace test_doubles

namespace {

class AISEC_MCDC_TrackCircuitInputTest_Decision_1 : public ::testing::Test {
protected:
    // Dependencies
    test_doubles::FakeGpio fake_gpio_;
    ::railway::drivers::TrackCircuitInput::Config config_;
    std::unique_ptr<::railway::drivers::TrackCircuitInput> track_circuit_input_;

    void SetUp() override {
        // Default configuration, can be overridden by specific tests
        config_.pin = 10;
        config_.activeLow = false; // Default: HIGH means NOT clear, LOW means clear
        config_.debounceMs = 50;
        config_.stuckLowFaultMs = 1000;

        // Initialize the class under test
        // init() is called here as per initialization instructions for init()-style methods
        track_circuit_input_ = std::make_unique<::railway::drivers::TrackCircuitInput>(config_, fake_gpio_);
        track_circuit_input_->init();
    }

    void TearDown() override {
        track_circuit_input_.reset();
    }

    // Helper to re-initialize the TrackCircuitInput with a new config
    void Reinit(const ::railway::drivers::TrackCircuitInput::Config& new_cfg) {
        config_ = new_cfg; // Update fixture's config
        track_circuit_input_ = std::make_unique<::railway::drivers::TrackCircuitInput>(config_, fake_gpio_);
        track_circuit_input_->init();
    }
};

// =====================================================================
// Tests for TrackCircuitInput::init()
// =====================================================================

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_InitConfiguresGpioAsInputPullup) {
    ASSERT_EQ(fake_gpio_.configured_pin_, config_.pin);
    ASSERT_EQ(fake_gpio_.configured_mode_, ::railway::hal::PinMode::InputPullup);
    ASSERT_EQ(fake_gpio_.configure_calls_, 1);
}

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_InitSetsInitialClearStateBasedOnGpioReadAndActiveLowConfig) {
    // Test Case 1: Default config (activeLow=false), GPIO reads LOW.
    // readRawClear() = activeLow ? rawHigh : !rawHigh
    //                = false ? (LOW==HIGH) : !(LOW==HIGH)
    //                = false ? false : !false = true (CLEAR)
    fake_gpio_.set_read_level(::railway::hal::PinLevel::Low);
    Reinit(config_); // Re-initialize with default (activeLow=false)
    ASSERT_FALSE(track_circuit_input_->isOccupied()); // Expect clear
    ASSERT_TRUE(track_circuit_input_->isHealthy());   // Expect healthy initially

    // Test Case 2: Config activeLow=true, GPIO reads HIGH.
    // readRawClear() = activeLow ? rawHigh : !rawHigh
    //                = true ? (HIGH==HIGH) : !(HIGH==HIGH)
    //                = true ? true : !true = true (CLEAR)
    ::railway::drivers::TrackCircuitInput::Config cfg_active_low_true = config_;
    cfg_active_low_true.activeLow = true;
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High);
    Reinit(cfg_active_low_true);
    ASSERT_FALSE(track_circuit_input_->isOccupied()); // Expect clear
    ASSERT_TRUE(track_circuit_input_->isHealthy());   // Expect healthy initially

    // Test Case 3: Config activeLow=false, GPIO reads HIGH.
    // readRawClear() = activeLow ? rawHigh : !rawHigh
    //                = false ? (HIGH==HIGH) : !(HIGH==HIGH)
    //                = false ? true : !true = false (NOT CLEAR / OCCUPIED)
    ::railway::drivers::TrackCircuitInput::Config cfg_active_low_false = config_;
    cfg_active_low_false.activeLow = false;
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High);
    Reinit(cfg_active_low_false);
    ASSERT_TRUE(track_circuit_input_->isOccupied()); // Expect occupied
    ASSERT_TRUE(track_circuit_input_->isHealthy());   // Expect healthy initially (fault timer not started yet)
}

// =====================================================================
// Tests for TrackCircuitInput::update()
// =====================================================================

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_UpdateDebouncesStateChangeFromClearToOccupied) {
    // Initial state (from SetUp): activeLow=false, gpio.read=Low -> CLEAR, !occupied
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy());

    ::railway::Millis now = 0; // Assume init at time 0

    // Simulate GPIO going HIGH (meaning NOT CLEAR/OCCUPIED with activeLow=false)
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High);
    now += 10; // Time advances
    track_circuit_input_->update(now);

    // Raw state has changed, but stable state is still CLEAR because debounce time not met
    ASSERT_FALSE(track_circuit_input_->isOccupied()); // Still reports clear
    ASSERT_TRUE(track_circuit_input_->isHealthy());

    // Advance time to just before debounce threshold
    now += (config_.debounceMs - 1); // now = 10 + 49 = 59
    track_circuit_input_->update(now);
    ASSERT_FALSE(track_circuit_input_->isOccupied()); // Still reports clear

    // Advance time past debounce threshold
    now += 1; // now = 60 (10 + 50)
    track_circuit_input_->update(now);
    ASSERT_TRUE(track_circuit_input_->isOccupied()); // Now reports occupied (stable state updated)
    ASSERT_TRUE(track_circuit_input_->isHealthy());
}

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_UpdateDebouncesStateChangeFromOccupiedToClear) {
    // First, make it stably occupied
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High); // Not clear
    ::railway::Millis now = 10;
    track_circuit_input_->update(now);
    now += config_.debounceMs;
    track_circuit_input_->update(now);
    ASSERT_TRUE(track_circuit_input_->isOccupied()); // Stably occupied

    // Simulate GPIO going LOW (meaning CLEAR with activeLow=false)
    fake_gpio_.set_read_level(::railway::hal::PinLevel::Low);
    now += 10; // Time advances
    track_circuit_input_->update(now);

    // Raw state changed, but stable state is still OCCUPIED
    ASSERT_TRUE(track_circuit_input_->isOccupied()); // Still reports occupied

    // Advance time past debounce threshold
    now += config_.debounceMs; // now = (10 + 50) + 10 + 50 = 120
    track_circuit_input_->update(now);
    ASSERT_FALSE(track_circuit_input_->isOccupied()); // Now reports clear
    ASSERT_TRUE(track_circuit_input_->isHealthy());
}

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_UpdateDetectsAndRecoversFromStuckLowFault) {
    // Initial state: clear, healthy
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy());

    ::railway::Millis now = 0;

    // 1. Make track stably occupied
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High); // Not clear
    now += 10;
    track_circuit_input_->update(now); // Raw change
    now += config_.debounceMs; // now = 10 + 50 = 60
    track_circuit_input_->update(now); // Stable change to occupied
    ASSERT_TRUE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy()); // Still healthy, fault timer just started

    // 2. Advance time past stuckLowFaultMs threshold
    now += config_.stuckLowFaultMs; // now = 60 + 1000 = 1060
    track_circuit_input_->update(now);
    ASSERT_TRUE(track_circuit_input_->isOccupied());
    ASSERT_FALSE(track_circuit_input_->isHealthy()); // Should be unhealthy now

    // 3. Track becomes clear again and stays clear past debounce
    fake_gpio_.set_read_level(::railway::hal::PinLevel::Low); // Clear
    now += 10; // now = 1070
    track_circuit_input_->update(now); // Raw change
    now += config_.debounceMs; // now = 1070 + 50 = 1120
    track_circuit_input_->update(now); // Stable change to clear
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy()); // Should be healthy again
}

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_UpdateDoesNotBecomeUnhealthyIfStuckLowClearsBeforeThreshold) {
    // Initial state: clear, healthy
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy());

    ::railway::Millis now = 0;

    // 1. Make track stably occupied
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High); // Not clear
    now += 10;
    track_circuit_input_->update(now);
    now += config_.debounceMs;
    track_circuit_input_->update(now);
    ASSERT_TRUE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy());

    // 2. Advance time, but not fully past stuckLowFaultMs (e.g., half way)
    now += (config_.stuckLowFaultMs / 2); // now = 60 + 500 = 560
    track_circuit_input_->update(now);
    ASSERT_TRUE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy()); // Still healthy

    // 3. Track becomes clear before fault threshold reached
    fake_gpio_.set_read_level(::railway::hal::PinLevel::Low); // Clear
    now += 10;
    track_circuit_input_->update(now);
    now += config_.debounceMs;
    track_circuit_input_->update(now);
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy()); // Remains healthy
}

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_UpdateHandlesZeroTimeThresholds) {
    ::railway::drivers::TrackCircuitInput::Config cfg_zero_times = config_;
    cfg_zero_times.debounceMs = 0;
    cfg_zero_times.stuckLowFaultMs = 0;
    Reinit(cfg_zero_times); // Re-initialize with zero debounce and fault times

    // Initial state: clear, healthy
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy());

    ::railway::Millis now = 100;

    // When GPIO goes HIGH (not clear), state should instantly become occupied and unhealthy
    fake_gpio_.set_read_level(::railway::hal::PinLevel::High);
    track_circuit_input_->update(now);
    ASSERT_TRUE(track_circuit_input_->isOccupied());
    ASSERT_FALSE(track_circuit_input_->isHealthy());

    // When GPIO goes LOW (clear), state should instantly become clear and healthy
    fake_gpio_.set_read_level(::railway::hal::PinLevel::Low);
    now += 10;
    track_circuit_input_->update(now);
    ASSERT_FALSE(track_circuit_input_->isOccupied());
    ASSERT_TRUE(track_circuit_input_->isHealthy());
}

// =====================================================================
// MC/DC Tests for line 23: return cfg_.activeLow ? rawHigh : !rawHigh;
// Decision: (cfg_.activeLow)
// Conditions:
// 1. cfg_.activeLow
//
// To achieve MC/DC, we need to show that condition 1 independently affects the outcome.
// This requires two test cases where `cfg_.activeLow` changes, while all other relevant conditions
// (in this case, the `rawHigh` value derived from `gpio_.read()`) remain constant, and the outcome changes.
// The outcome of this expression determines the initial `rawClear_` and `stableClear_` in `init()`,
// which we can observe via `isOccupied()`.
// =====================================================================

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_McDc_ReadRawClear_ActiveLowTrue_GpioHigh_ExpectClear) {
    // MC/DC Condition: cfg_.activeLow = TRUE
    // Other condition: rawHigh = TRUE (i.e., gpio.read() returns HIGH)
    ::railway::drivers::TrackCircuitInput::Config mcdc_config = config_;
    mcdc_config.activeLow = true; // Set activeLow to TRUE

    fake_gpio_.set_read_level(::railway::hal::PinLevel::High); // Set GPIO to HIGH (rawHigh will be true)

    Reinit(mcdc_config); // Re-initialize with this configuration

    // Expected outcome for readRawClear():
    // `true ? true : !true` -> `true` (CLEAR)
    ASSERT_FALSE(track_circuit_input_->isOccupied()); // Asserting that the initial state is CLEAR
    ASSERT_TRUE(track_circuit_input_->isHealthy());
}

TEST_F(AISEC_MCDC_TrackCircuitInputTest_Decision_1, AISEC_MCDC_McDc_ReadRawClear_ActiveLowFalse_GpioHigh_ExpectOccupied) {
    // MC/DC Condition: cfg_.activeLow = FALSE (differs from above)
    // Other condition: rawHigh = TRUE (i.e., gpio.read() returns HIGH - kept constant)
    ::railway::drivers::TrackCircuitInput::Config mcdc_config = config_;
    mcdc_config.activeLow = false; // Set activeLow to FALSE

    fake_gpio_.set_read_level(::railway::hal::PinLevel::High); // Set GPIO to HIGH (rawHigh will be true - same as above)

    Reinit(mcdc_config); // Re-initialize with this configuration

    // Expected outcome for readRawClear():
    // `false ? true : !true` -> `false` (NOT CLEAR / OCCUPIED)
    ASSERT_TRUE(track_circuit_input_->isOccupied()); // Asserting that the initial state is OCCUPIED
    ASSERT_TRUE(track_circuit_input_->isHealthy());
}

} // namespace

/*
Not directly included (hardware-touching):
- None directly from the `TrackCircuitInput` class. The `gpio_` dependency is an interface and is faked.

Stubbed/External:
- railway::hal::IGpio (interface, faked for testing as railway::hal::FakeGpio)
- Arduino_stubs.h (provides `railway::Millis` as `uint32_t` and other common Arduino types)
*/
// Not directly included (needs stubs/external/too deep): configure, readRawClear

}  // namespace

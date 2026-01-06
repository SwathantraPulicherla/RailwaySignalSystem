add_executable(railway_tests
    test_interlocking.cpp
)

target_link_libraries(railway_tests PRIVATE railway_logic)
target_compile_features(railway_tests PRIVATE cxx_std_17)

# Support either a system-provided GTest or the FetchContent one.
if(TARGET GTest::gtest_main)
    target_link_libraries(railway_tests PRIVATE GTest::gtest_main)
elseif(TARGET gtest_main)
    target_link_libraries(railway_tests PRIVATE gtest_main)
else()
    message(FATAL_ERROR "GoogleTest not found/built; cannot link railway_tests. Configure so find_package(GTest) succeeds, or enable -DRAILWAY_FETCH_GTEST=ON.")
endif()

include(GoogleTest)
# gtest_discover_tests(railway_tests)

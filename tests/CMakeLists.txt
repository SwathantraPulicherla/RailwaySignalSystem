file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "test_*.cpp"
)

# Optional host stubs used by generated tests (e.g. Arduino_stubs.h).
set(TEST_STUBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stubs")

# Exclude non-test artifact directories under tests/.
set(_FILTERED_TEST_SOURCES "")
foreach(_p ${TEST_SOURCES})
    string(REPLACE "\\" "/" _p_norm "${_p}")
    if(_p_norm MATCHES "(^|/)(build|review|compilation_report|test_reports|coverage_report)/")
        # skip
    else()
        list(APPEND _FILTERED_TEST_SOURCES "${_p}")
    endif()
endforeach()
set(TEST_SOURCES ${_FILTERED_TEST_SOURCES})

# Build each test source as its own executable so CTest can run/filter by filename
# (e.g. `ctest -R test_ControllerHelpers`).
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Normalize to an absolute path for name derivation and add_executable().
    if(IS_ABSOLUTE "${TEST_SOURCE}")
        set(_abs "${TEST_SOURCE}")
    else()
        set(_abs "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE}")
    endif()

    # Use a unique test/executable name derived from its folder, while keeping the basename
    # in the name so `ctest -R test_Foo` still works.
    file(RELATIVE_PATH _rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_abs}")
    get_filename_component(_stem "${_abs}" NAME_WE)
    get_filename_component(_dir "${_rel}" DIRECTORY)
    if(_dir STREQUAL "")
        set(TEST_NAME "${_stem}")
    else()
        string(REPLACE "/" "__" _dir_safe "${_dir}")
        set(TEST_NAME "${_stem}__${_dir_safe}")
    endif()

    add_executable(${TEST_NAME}
        ${_abs}
    )

    if(EXISTS "${TEST_STUBS_DIR}/Arduino_stubs.cpp")
        target_sources(${TEST_NAME} PRIVATE "${TEST_STUBS_DIR}/Arduino_stubs.cpp")
    endif()
    if(EXISTS "${TEST_STUBS_DIR}")
        target_include_directories(${TEST_NAME} PRIVATE "${TEST_STUBS_DIR}")
    endif()

    target_link_libraries(${TEST_NAME} PRIVATE railway_logic)
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_17)

    # Support either a system-provided GTest or the FetchContent one.
    if(TARGET GTest::gtest_main)
        target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main)
    elseif(TARGET gtest_main)
        target_link_libraries(${TEST_NAME} PRIVATE gtest_main)
    else()
        message(FATAL_ERROR "GoogleTest not found/built; cannot link ${TEST_NAME}. Configure so find_package(GTest) succeeds, or enable -DRAILWAY_FETCH_GTEST=ON.")
    endif()

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

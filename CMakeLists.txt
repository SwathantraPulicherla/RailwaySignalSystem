cmake_minimum_required(VERSION 3.14)

project(railway_system)

set(CMAKE_CXX_STANDARD 17)

# Optional coverage instrumentation (requires GCC/Clang + gcov-compatible tooling).
option(RAILWAY_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
if(RAILWAY_ENABLE_COVERAGE)
  if(MSVC)
    message(WARNING "RAILWAY_ENABLE_COVERAGE is ON but MSVC is in use; gcov-based coverage will not work.")
  else()
    add_compile_options(--coverage)
    add_link_options(--coverage)
  endif()
endif()

# Enable CTest (so `ctest` discovers tests added via add_test())
include(CTest)
enable_testing()

include_directories(include)

# Google Test setup
option(RAILWAY_FETCH_GTEST "Fetch GoogleTest" OFF)
if(RAILWAY_FETCH_GTEST)
  set(CMAKE_TLS_VERIFY 0)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(src)

add_subdirectory(tests)

add_test(NAME test_Interlocking COMMAND ${CMAKE_BINARY_DIR}/tests/railway_tests)

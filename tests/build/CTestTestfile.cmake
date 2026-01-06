# CMake generated Testfile for 
# Source directory: C:/Users/SwathantraPulicherla/workspaces/UnitTestGen/RailwaySignalSystem
# Build directory: C:/Users/SwathantraPulicherla/workspaces/UnitTestGen/RailwaySignalSystem/tests/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_Interlocking "C:/Users/SwathantraPulicherla/workspaces/UnitTestGen/RailwaySignalSystem/tests/build/tests/railway_tests")
set_tests_properties(test_Interlocking PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/SwathantraPulicherla/workspaces/UnitTestGen/RailwaySignalSystem/CMakeLists.txt;31;add_test;C:/Users/SwathantraPulicherla/workspaces/UnitTestGen/RailwaySignalSystem/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
subdirs("src")
subdirs("tests")

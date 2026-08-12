# CMake generated Testfile for 
# Source directory: C:/STMProjects/tomko_lib/can_database/test
# Build directory: C:/STMProjects/tomko_lib/build/ci/can_database/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[test_can_database]=] "C:/STMProjects/tomko_lib/build/ci/bin/Debug/test_can_database.exe")
  set_tests_properties([=[test_can_database]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;13;add_test;C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[test_can_database]=] "C:/STMProjects/tomko_lib/build/ci/bin/Release/test_can_database.exe")
  set_tests_properties([=[test_can_database]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;13;add_test;C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[test_can_database]=] "C:/STMProjects/tomko_lib/build/ci/bin/MinSizeRel/test_can_database.exe")
  set_tests_properties([=[test_can_database]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;13;add_test;C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[test_can_database]=] "C:/STMProjects/tomko_lib/build/ci/bin/RelWithDebInfo/test_can_database.exe")
  set_tests_properties([=[test_can_database]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;13;add_test;C:/STMProjects/tomko_lib/can_database/test/CMakeLists.txt;0;")
else()
  add_test([=[test_can_database]=] NOT_AVAILABLE)
endif()

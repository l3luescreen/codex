add_executable(LookupHistoryTest
  LookupHistoryTest.cpp
  ${REPO_ROOT}/src/util/LookupHistoryFormat.cpp
)

target_include_directories(LookupHistoryTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(LookupHistoryTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(LookupHistoryTest)

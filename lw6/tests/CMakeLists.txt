file(GLOB_RECURSE TEST_SRC "*.h" "*.cpp")
add_executable(tests ${TEST_SRC})

target_link_libraries(tests
        PRIVATE
        gtest
        gtest_main
        gmock
        gmock_main
)

include(GoogleTest)
gtest_discover_tests(tests)
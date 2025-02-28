cmake_minimum_required(VERSION 3.22.1)
project(lw1)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(task1_1)
add_subdirectory(task1_2)

#include(FetchContent)
#FetchContent_Declare(
#        googletest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        GIT_TAG release-1.12.1
#)
#FetchContent_MakeAvailable(googletest)
# add_subdirectory(tests)
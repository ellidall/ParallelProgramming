cmake_minimum_required(VERSION 3.22.1)
project(lw2)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(task2_1)

#include(FetchContent)
#FetchContent_Declare(
#        googletest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        GIT_TAG release-1.12.1
#)
#FetchContent_MakeAvailable(googletest)
# add_subdirectory(tests)
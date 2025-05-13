file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(atomicMax ${SRC})

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(
        ${Boost_INCLUDE_DIRS}
)
target_link_libraries(atomicMax
        ${Boost_LIBRARIES}
)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task6_2/bin)
add_custom_command(
        TARGET atomicMax POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:atomicMax> ${CMAKE_SOURCE_DIR}/task6_2/bin
        COMMENT "Копируем исполнимый файл в task6_2/bin"
)

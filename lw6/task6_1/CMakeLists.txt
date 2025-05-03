file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(ticket ${SRC})

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(
        ${Boost_INCLUDE_DIRS}
)
target_link_libraries(ticket
        ${Boost_LIBRARIES}
)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task6_1/bin)
add_custom_command(
        TARGET ticket POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ticket> ${CMAKE_SOURCE_DIR}/task6_1/bin
        COMMENT "Копируем исполнимый файл в task6_1/bin"
)

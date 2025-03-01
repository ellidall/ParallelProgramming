file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(life ${SRC})

target_link_libraries(life sfml-graphics sfml-window sfml-system)

add_custom_command(TARGET life POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:life> ${CMAKE_SOURCE_DIR}/task2_1
        COMMENT "Copying executable to task directory"
)
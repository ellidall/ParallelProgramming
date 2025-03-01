file(GLOB_RECURSE SRC "*.h" "*.cpp")
add_executable(life ${SRC})

add_custom_command(TARGET life POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:life> ${CMAKE_SOURCE_DIR}/task2_1
        COMMENT "Copying executable to task directory"
)
file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(gauss ${SRC})

target_link_libraries(gauss sfml-graphics sfml-window sfml-system)

add_custom_command(TARGET gauss POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gauss> ${CMAKE_SOURCE_DIR}/task2_2/bin
        COMMENT "Copying executable to task directory"
)
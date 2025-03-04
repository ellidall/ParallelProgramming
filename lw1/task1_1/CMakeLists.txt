file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(archiver ${SRC})

target_link_libraries(archiver sfml-graphics sfml-window sfml-system)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task1_1/bin)
add_custom_command(
        TARGET archiver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:archiver> ${CMAKE_SOURCE_DIR}/task1_1/bin
        COMMAND ${CMAKE_COMMAND} -E echo "Target file: $<TARGET_FILE:archiver>"
        COMMENT "Copying executable to task directory"
)
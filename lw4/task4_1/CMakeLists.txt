file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(music ${SRC})

target_link_libraries(music sfml-audio sfml-system)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task4_1/bin)
add_custom_command(
        TARGET music POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:music> ${CMAKE_SOURCE_DIR}/task4_1/bin
        COMMENT "Copying executable to task directory"
)
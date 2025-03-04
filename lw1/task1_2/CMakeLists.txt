file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(extractor ${SRC})

target_link_libraries(extractor sfml-graphics sfml-window sfml-system)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task1_2/bin)
add_custom_command(TARGET extractor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:extractor> ${CMAKE_SOURCE_DIR}/task1_2/bin
        COMMENT "Copying executable to task directory"
)
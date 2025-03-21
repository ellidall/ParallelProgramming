file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(bank ${SRC})

target_link_libraries(bank sfml-graphics sfml-window sfml-system)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task3_1/bin)
add_custom_command(
        TARGET bank POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bank> ${CMAKE_SOURCE_DIR}/task3_1/bin
        COMMENT "Copying executable to task directory"
)
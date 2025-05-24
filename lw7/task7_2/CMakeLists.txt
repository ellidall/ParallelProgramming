file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(book ${SRC})

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task7_2/bin)
add_custom_command(
        TARGET book POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:book> ${CMAKE_SOURCE_DIR}/task7_2/bin
        COMMENT "Копируем исполнимый файл в task7_2/bin"
)
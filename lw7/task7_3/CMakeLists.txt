file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(awaiter ${SRC})

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task7_3/bin)
add_custom_command(
        TARGET awaiter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:awaiter> ${CMAKE_SOURCE_DIR}/task7_3/bin
        COMMENT "Копируем исполнимый файл в task7_3/bin"
)

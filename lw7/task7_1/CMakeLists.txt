file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(myTask ${SRC})

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task7_1/bin)
add_custom_command(
        TARGET myTask POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:myTask> ${CMAKE_SOURCE_DIR}/task7_1/bin
        COMMENT "Копируем исполнимый файл в task7_1/bin"
)

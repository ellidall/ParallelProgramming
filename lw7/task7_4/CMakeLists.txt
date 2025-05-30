file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(async ${SRC})


find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)
target_link_libraries(async PRIVATE ${LIBURING_LIBRARIES})
if (LIBURING_FOUND)
    target_include_directories(async PRIVATE ${LIBURING_INCLUDE_DIRS})
endif()


file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task7_4/bin)
add_custom_command(
        TARGET async POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:async> ${CMAKE_SOURCE_DIR}/task7_4/bin
        COMMENT "Копируем исполнимый файл в task7_4/bin"
)

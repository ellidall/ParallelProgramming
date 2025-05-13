file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(queue ${SRC})

find_package(Boost 1.66.0 REQUIRED COMPONENTS thread system)
include_directories(
        ${Boost_INCLUDE_DIRS}
)
target_link_libraries(queue
        PRIVATE
        Boost::boost
        Boost::thread
)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task6_5/bin)
add_custom_command(
        TARGET queue POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:queue> ${CMAKE_SOURCE_DIR}/task6_5/bin
        COMMENT "Копируем исполнимый файл в task6_5/bin"
)

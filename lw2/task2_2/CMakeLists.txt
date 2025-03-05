file(GLOB_RECURSE SRC "*.h" "*.cpp")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
add_executable(gauss ${SRC})

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(gauss ${OpenCV_LIBS})

target_link_libraries(gauss sfml-graphics sfml-window sfml-system)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task2_2/bin)
add_custom_command(TARGET gauss POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gauss> ${CMAKE_SOURCE_DIR}/task2_2/bin
        COMMENT "Copying executable to task directory"
)
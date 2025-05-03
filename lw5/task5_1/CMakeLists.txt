file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(tv ${SRC}
        Television.cpp
        Server.h)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${PORTAUDIO_INCLUDE_DIRS}
)
target_link_libraries(tv
        ${OpenCV_LIBS}
        ${Boost_LIBRARIES}
        ${PORTAUDIO_LIBRARIES}
        pthread
)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task5_1/bin)
add_custom_command(
        TARGET tv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tv> ${CMAKE_SOURCE_DIR}/task5_1/bin
        COMMENT "Копируем исполнимый файл в task5_1/bin"
)

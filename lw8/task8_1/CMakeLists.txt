file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(bitonicSort ${SRC})

include_directories(
        "C:/Users/alexa/Downloads/OpenCL-Headers"
        "C:/Users/alexa/Downloads/OpenCL-CLHPP/include"
)
target_link_libraries(bitonicSort OpenCL)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task8_1/bin)
add_custom_command(
        TARGET bitonicSort POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bitonicSort> ${CMAKE_SOURCE_DIR}/task8_1/bin
        COMMENT "Копируем исполнимый файл в task8_1/bin"
)

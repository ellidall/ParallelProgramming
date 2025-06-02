set(OpenCLHeaders_DIR "C:/OpenCL/opencl-headers-install/share/cmake/OpenCLHeaders")
set(OpenCLICDLoader_DIR "C:/Program Files (x86)/OpenCL-ICD-Loader/share/cmake/OpenCLICDLoader")

find_package(OpenCLHeaders REQUIRED)
find_package(OpenCLICDLoader REQUIRED)

include_directories(${OpenCLHeaders_INCLUDE_DIRS})
link_directories(${OpenCLICDLoader_LIBRARY_DIRS})

file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(gauss ${SRC})

target_include_directories(gauss PRIVATE
        C:/OpenCL/opencl-headers-install/include
        C:/OpenCL/OpenCL-CLHPP/include
)
target_link_libraries(gauss "C:/Program Files (x86)/OpenCL-ICD-Loader/lib/libOpenCL.dll.a")

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task8_2/bin)
add_custom_command(
        TARGET gauss POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gauss> ${CMAKE_SOURCE_DIR}/task8_2/bin
        COMMENT "Копируем исполнимый файл в task8_2/bin"
)

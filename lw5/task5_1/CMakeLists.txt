# Собираем все исходные файлы
file(GLOB_RECURSE SRC "*.cpp" "*.h")

# Создаем исполняемый файл
add_executable(tv ${SRC})

# === Boost ===
set(BOOST_ROOT "C:/Sasha/Studying/Libs/boost_1_88_0")
find_package(Boost REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost found at ${Boost_INCLUDE_DIRS}")
    target_include_directories(tv PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(tv PRIVATE ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# === OpenCV ===
set(OpenCV_DIR "C:/Sasha/Studying/Libs/opencv/build")
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    target_include_directories(tv PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(tv PRIVATE ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# === SFML ===
find_package(SFML 2.5 REQUIRED COMPONENTS audio system)
if(SFML_FOUND)
    target_include_directories(tv PRIVATE ${SFML_INCLUDE_DIR})
    target_link_libraries(tv PRIVATE sfml-audio sfml-system)
else()
    message(FATAL_ERROR "SFML not found!")
endif()

# === Копирование исполняемого файла ===
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/task5_1/bin)
add_custom_command(
        TARGET tv POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tv> ${CMAKE_SOURCE_DIR}/task5_1/bin
        COMMENT "Копируем исполнимый файл в task5_1/bin"
)

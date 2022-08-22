cmake_minimum_required(VERSION 3.5)

project(beshray)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    src/main.c++
    src/map.c++
    src/engine.c++
    src/texture.c++
    src/camera.c++
    src/framebuffer.c++
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME}
    PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic 
)

find_package(SFML COMPONENTS system window audio graphics REQUIRED)

target_link_libraries(${PROJECT_NAME} sfml-graphics)
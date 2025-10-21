cmake_minimum_required(VERSION 3.27)

project(ch8)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/chip8.c
    src/gfx.c
)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

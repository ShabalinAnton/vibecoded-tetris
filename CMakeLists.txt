cmake_minimum_required(VERSION 3.15)
project(tetris VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(tetris
    src/main.cpp
    src/Game.cpp
    src/Board.cpp
    src/Tetromino.cpp
    src/Renderer.cpp
    src/Sound.cpp
    src/Music.cpp
)

target_include_directories(tetris PRIVATE src)
target_link_libraries(tetris PRIVATE SDL2::SDL2 SDL2::SDL2main)

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A simple Tetris game built with C++, CMake, and Conan package manager.

## Build Commands

```bash
# Install dependencies
conan install . --output-folder=build --build=missing

# Configure and build (using CMake presets)
cmake --preset conan-release
cmake --build --preset conan-release

# Run
./build/build/Release/tetris
```

## Dependencies

- **SDL2**: Graphics, window management, and input handling (via Conan)
- **C++17**: Modern C++ features

## Architecture

```
src/
├── main.cpp          # Entry point
├── Game.cpp/h        # Game state, main loop, input handling
├── Board.cpp/h       # 10x20 grid, collision detection, line clearing
├── Tetromino.cpp/h   # 7 piece types (I,O,T,S,Z,J,L), rotation states
├── Renderer.cpp/h    # SDL2 rendering, draw board and pieces
├── Sound.cpp/h       # Procedural audio generation for sound effects
└── Music.cpp/h       # Procedural EDM background music (128 BPM)
```

## Game Controls

- **Left/Right Arrow**: Move piece horizontally
- **Down Arrow**: Soft drop (faster fall)
- **Up Arrow**: Rotate piece clockwise
- **Space**: Hard drop (instant fall)
- **Escape**: Quit game
- **Enter/Space**: Restart after game over

## Key Constants

- Board: 10 columns × 20 rows
- Initial drop speed: 1 row per 500ms
- Level up every 10 lines cleared
- Classic Tetris scoring: 100/300/500/800 points for 1/2/3/4 lines × level

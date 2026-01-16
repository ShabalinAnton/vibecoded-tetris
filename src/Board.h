#pragma once

#include "Tetromino.h"
#include <array>
#include <optional>

class Board {
public:
    static constexpr int WIDTH = 10;
    static constexpr int HEIGHT = 20;

    Board();

    bool isValidPosition(const Tetromino& piece) const;
    void placePiece(const Tetromino& piece);
    int clearLines();

    std::optional<TetrominoType> getCell(int x, int y) const;
    bool isEmpty(int x, int y) const;

    void clear();

private:
    // Store the type of tetromino in each cell (for coloring)
    // nullopt means empty
    std::array<std::array<std::optional<TetrominoType>, WIDTH>, HEIGHT> grid_;
};

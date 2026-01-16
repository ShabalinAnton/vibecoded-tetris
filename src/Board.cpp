#include "Board.h"

Board::Board() {
    clear();
}

void Board::clear() {
    for (auto& row : grid_) {
        row.fill(std::nullopt);
    }
}

bool Board::isValidPosition(const Tetromino& piece) const {
    const auto& shape = piece.getShape();
    int pieceX = piece.getX();
    int pieceY = piece.getY();

    for (int y = 0; y < Tetromino::SIZE; y++) {
        for (int x = 0; x < Tetromino::SIZE; x++) {
            if (!shape[y][x]) continue;

            int boardX = pieceX + x;
            int boardY = pieceY + y;

            // Check bounds
            if (boardX < 0 || boardX >= WIDTH) return false;
            if (boardY >= HEIGHT) return false;
            // Allow pieces above the board (negative Y)
            if (boardY < 0) continue;

            // Check collision with placed pieces
            if (grid_[boardY][boardX].has_value()) return false;
        }
    }
    return true;
}

void Board::placePiece(const Tetromino& piece) {
    const auto& shape = piece.getShape();
    int pieceX = piece.getX();
    int pieceY = piece.getY();

    for (int y = 0; y < Tetromino::SIZE; y++) {
        for (int x = 0; x < Tetromino::SIZE; x++) {
            if (!shape[y][x]) continue;

            int boardX = pieceX + x;
            int boardY = pieceY + y;

            if (boardY >= 0 && boardY < HEIGHT && boardX >= 0 && boardX < WIDTH) {
                grid_[boardY][boardX] = piece.getType();
            }
        }
    }
}

int Board::clearLines() {
    int linesCleared = 0;

    for (int y = HEIGHT - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < WIDTH; x++) {
            if (!grid_[y][x].has_value()) {
                fullLine = false;
                break;
            }
        }

        if (fullLine) {
            linesCleared++;
            // Move all lines above down
            for (int moveY = y; moveY > 0; moveY--) {
                grid_[moveY] = grid_[moveY - 1];
            }
            // Clear top line
            grid_[0].fill(std::nullopt);
            // Check this line again since we moved everything down
            y++;
        }
    }

    return linesCleared;
}

std::optional<TetrominoType> Board::getCell(int x, int y) const {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return std::nullopt;
    }
    return grid_[y][x];
}

bool Board::isEmpty(int x, int y) const {
    return !getCell(x, y).has_value();
}

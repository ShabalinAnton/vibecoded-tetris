#include "Tetromino.h"

Tetromino::Tetromino(TetrominoType type) : type_(type) {
    initShapes();
}

void Tetromino::rotateClockwise() {
    rotation_ = (rotation_ + 1) % 4;
}

void Tetromino::rotateCounterClockwise() {
    rotation_ = (rotation_ + 3) % 4;
}

Color Tetromino::getColor() const {
    return getColorForType(type_);
}

Color Tetromino::getColorForType(TetrominoType type) {
    switch (type) {
        case TetrominoType::I: return {0, 255, 255};   // Cyan
        case TetrominoType::O: return {255, 255, 0};   // Yellow
        case TetrominoType::T: return {128, 0, 128};   // Purple
        case TetrominoType::S: return {0, 255, 0};     // Green
        case TetrominoType::Z: return {255, 0, 0};     // Red
        case TetrominoType::J: return {0, 0, 255};     // Blue
        case TetrominoType::L: return {255, 165, 0};   // Orange
        default: return {255, 255, 255};
    }
}

void Tetromino::initShapes() {
    // Initialize all rotations to empty
    for (auto& shape : shapes_) {
        for (auto& row : shape) {
            row.fill(false);
        }
    }

    // Define base shape (rotation 0)
    switch (type_) {
        case TetrominoType::I:
            // ....
            // XXXX
            // ....
            // ....
            shapes_[0][1][0] = shapes_[0][1][1] = shapes_[0][1][2] = shapes_[0][1][3] = true;
            break;

        case TetrominoType::O:
            // ....
            // .XX.
            // .XX.
            // ....
            shapes_[0][1][1] = shapes_[0][1][2] = shapes_[0][2][1] = shapes_[0][2][2] = true;
            break;

        case TetrominoType::T:
            // ....
            // XXX.
            // .X..
            // ....
            shapes_[0][1][0] = shapes_[0][1][1] = shapes_[0][1][2] = true;
            shapes_[0][2][1] = true;
            break;

        case TetrominoType::S:
            // ....
            // .XX.
            // XX..
            // ....
            shapes_[0][1][1] = shapes_[0][1][2] = true;
            shapes_[0][2][0] = shapes_[0][2][1] = true;
            break;

        case TetrominoType::Z:
            // ....
            // XX..
            // .XX.
            // ....
            shapes_[0][1][0] = shapes_[0][1][1] = true;
            shapes_[0][2][1] = shapes_[0][2][2] = true;
            break;

        case TetrominoType::J:
            // ....
            // XXX.
            // ..X.
            // ....
            shapes_[0][1][0] = shapes_[0][1][1] = shapes_[0][1][2] = true;
            shapes_[0][2][2] = true;
            break;

        case TetrominoType::L:
            // ....
            // XXX.
            // X...
            // ....
            shapes_[0][1][0] = shapes_[0][1][1] = shapes_[0][1][2] = true;
            shapes_[0][2][0] = true;
            break;

        default:
            break;
    }

    // Generate rotations by rotating the base shape
    for (int r = 1; r < 4; r++) {
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                // Rotate 90 degrees clockwise: new[x][SIZE-1-y] = old[y][x]
                shapes_[r][x][SIZE - 1 - y] = shapes_[r - 1][y][x];
            }
        }
    }

    // O piece doesn't rotate (all rotations are the same)
    if (type_ == TetrominoType::O) {
        for (int r = 1; r < 4; r++) {
            shapes_[r] = shapes_[0];
        }
    }
}

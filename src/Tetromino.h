#pragma once

#include <array>
#include <cstdint>

enum class TetrominoType {
    I, O, T, S, Z, J, L,
    Count
};

struct Color {
    uint8_t r, g, b;
};

class Tetromino {
public:
    static constexpr int SIZE = 4;
    using Shape = std::array<std::array<bool, SIZE>, SIZE>;

    explicit Tetromino(TetrominoType type);

    void rotateClockwise();
    void rotateCounterClockwise();

    const Shape& getShape() const { return shapes_[rotation_]; }
    TetrominoType getType() const { return type_; }
    Color getColor() const;

    int getX() const { return x_; }
    int getY() const { return y_; }
    void setPosition(int x, int y) { x_ = x; y_ = y; }
    void move(int dx, int dy) { x_ += dx; y_ += dy; }

    static Color getColorForType(TetrominoType type);

private:
    void initShapes();

    TetrominoType type_;
    int rotation_ = 0;
    int x_ = 0;
    int y_ = 0;
    std::array<Shape, 4> shapes_;
};

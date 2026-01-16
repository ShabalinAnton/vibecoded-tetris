#include "Renderer.h"
#include <iostream>
#include <cstring>

// 5x7 bitmap font for digits 0-9
// Each digit is represented as 7 rows of 5 bits
static const uint8_t DIGIT_FONT[10][7] = {
    {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}, // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}, // 1
    {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111}, // 2
    {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110}, // 3
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}, // 4
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}, // 5
    {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}, // 6
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}, // 7
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}, // 8
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}, // 9
};

// Simple 5x7 bitmap font for letters (uppercase only, subset)
static const uint8_t LETTER_FONT[26][7] = {
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // A
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}, // B
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}, // C
    {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}, // D
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}, // E
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}, // F
    {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111}, // G
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // H
    {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}, // I
    {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100}, // J
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}, // K
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}, // L
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}, // M
    {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001}, // N
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // O
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}, // P
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}, // Q
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}, // R
    {0b01110, 0b10001, 0b10000, 0b01110, 0b00001, 0b10001, 0b01110}, // S
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}, // T
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // U
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}, // V
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}, // W
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}, // X
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}, // Y
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}, // Z
};

Renderer::Renderer() {
    boardOffsetX_ = PADDING;
    boardOffsetY_ = PADDING;
}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    int windowWidth = Board::WIDTH * CELL_SIZE + PADDING * 2 + SIDEBAR_WIDTH;
    int windowHeight = Board::HEIGHT * CELL_SIZE + PADDING * 2;

    window_ = SDL_CreateWindow(
        "Tetris",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_SHOWN
    );

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void Renderer::shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer_, 20, 20, 20, 255);
    SDL_RenderClear(renderer_);

    // Draw board background
    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_Rect boardRect = {
        boardOffsetX_,
        boardOffsetY_,
        Board::WIDTH * CELL_SIZE,
        Board::HEIGHT * CELL_SIZE
    };
    SDL_RenderFillRect(renderer_, &boardRect);

    // Draw grid lines
    SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255);
    for (int x = 0; x <= Board::WIDTH; x++) {
        SDL_RenderDrawLine(
            renderer_,
            boardOffsetX_ + x * CELL_SIZE, boardOffsetY_,
            boardOffsetX_ + x * CELL_SIZE, boardOffsetY_ + Board::HEIGHT * CELL_SIZE
        );
    }
    for (int y = 0; y <= Board::HEIGHT; y++) {
        SDL_RenderDrawLine(
            renderer_,
            boardOffsetX_, boardOffsetY_ + y * CELL_SIZE,
            boardOffsetX_ + Board::WIDTH * CELL_SIZE, boardOffsetY_ + y * CELL_SIZE
        );
    }
}

void Renderer::present() {
    SDL_RenderPresent(renderer_);
}

void Renderer::drawCell(int x, int y, Color color, int offsetX, int offsetY) {
    SDL_Rect rect = {
        offsetX + x * CELL_SIZE + 1,
        offsetY + y * CELL_SIZE + 1,
        CELL_SIZE - 2,
        CELL_SIZE - 2
    };

    // Draw filled cell
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer_, &rect);

    // Draw highlight (top and left edges)
    SDL_SetRenderDrawColor(renderer_,
        std::min(255, color.r + 50),
        std::min(255, color.g + 50),
        std::min(255, color.b + 50),
        255);
    SDL_RenderDrawLine(renderer_, rect.x, rect.y, rect.x + rect.w - 1, rect.y);
    SDL_RenderDrawLine(renderer_, rect.x, rect.y, rect.x, rect.y + rect.h - 1);

    // Draw shadow (bottom and right edges)
    SDL_SetRenderDrawColor(renderer_,
        std::max(0, color.r - 50),
        std::max(0, color.g - 50),
        std::max(0, color.b - 50),
        255);
    SDL_RenderDrawLine(renderer_, rect.x, rect.y + rect.h - 1, rect.x + rect.w - 1, rect.y + rect.h - 1);
    SDL_RenderDrawLine(renderer_, rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1);
}

void Renderer::drawDigit(int digit, int x, int y, int scale) {
    if (digit < 0 || digit > 9) return;

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (DIGIT_FONT[digit][row] & (1 << (4 - col))) {
                SDL_Rect pixel = {
                    x + col * scale,
                    y + row * scale,
                    scale,
                    scale
                };
                SDL_RenderFillRect(renderer_, &pixel);
            }
        }
    }
}

void Renderer::drawNumber(int number, int x, int y, int scale, int minDigits) {
    // Handle zero
    if (number == 0 && minDigits <= 1) {
        drawDigit(0, x, y, scale);
        return;
    }

    // Count digits
    int temp = number;
    int digitCount = 0;
    while (temp > 0) {
        digitCount++;
        temp /= 10;
    }

    // Pad with leading zeros if needed
    if (digitCount < minDigits) {
        digitCount = minDigits;
    }

    // Draw digits from right to left
    int digitWidth = 6 * scale; // 5 pixels + 1 spacing
    int currentX = x + (digitCount - 1) * digitWidth;

    if (number == 0) {
        for (int i = 0; i < minDigits; i++) {
            drawDigit(0, x + i * digitWidth, y, scale);
        }
        return;
    }

    temp = number;
    int drawn = 0;
    while (temp > 0 || drawn < minDigits) {
        int digit = temp % 10;
        drawDigit(digit, currentX, y, scale);
        temp /= 10;
        currentX -= digitWidth;
        drawn++;
    }
}

void Renderer::drawLabel(const char* label, int x, int y) {
    int scale = 2;
    int charWidth = 6 * scale;

    SDL_SetRenderDrawColor(renderer_, 150, 150, 150, 255);

    for (int i = 0; label[i] != '\0'; i++) {
        char c = label[i];
        if (c >= 'A' && c <= 'Z') {
            int idx = c - 'A';
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (LETTER_FONT[idx][row] & (1 << (4 - col))) {
                        SDL_Rect pixel = {
                            x + i * charWidth + col * scale,
                            y + row * scale,
                            scale,
                            scale
                        };
                        SDL_RenderFillRect(renderer_, &pixel);
                    }
                }
            }
        } else if (c >= 'a' && c <= 'z') {
            int idx = c - 'a';
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (LETTER_FONT[idx][row] & (1 << (4 - col))) {
                        SDL_Rect pixel = {
                            x + i * charWidth + col * scale,
                            y + row * scale,
                            scale,
                            scale
                        };
                        SDL_RenderFillRect(renderer_, &pixel);
                    }
                }
            }
        }
    }
}

void Renderer::drawTime(int totalSeconds, int x, int y) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    int scale = 2;
    int digitWidth = 6 * scale;

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);

    // Draw minutes (at least 2 digits)
    drawNumber(minutes, x, y, scale, 2);

    // Draw colon
    int colonX = x + 2 * digitWidth + 2;
    SDL_Rect dot1 = {colonX, y + 2 * scale, scale, scale};
    SDL_Rect dot2 = {colonX, y + 5 * scale, scale, scale};
    SDL_RenderFillRect(renderer_, &dot1);
    SDL_RenderFillRect(renderer_, &dot2);

    // Draw seconds (always 2 digits)
    drawNumber(seconds, x + 2 * digitWidth + scale + 4, y, scale, 2);
}

void Renderer::drawBoard(const Board& board) {
    for (int y = 0; y < Board::HEIGHT; y++) {
        for (int x = 0; x < Board::WIDTH; x++) {
            auto cell = board.getCell(x, y);
            if (cell.has_value()) {
                Color color = Tetromino::getColorForType(cell.value());
                drawCell(x, y, color, boardOffsetX_, boardOffsetY_);
            }
        }
    }
}

void Renderer::drawPiece(const Tetromino& piece) {
    const auto& shape = piece.getShape();
    Color color = piece.getColor();

    for (int y = 0; y < Tetromino::SIZE; y++) {
        for (int x = 0; x < Tetromino::SIZE; x++) {
            if (shape[y][x]) {
                int boardX = piece.getX() + x;
                int boardY = piece.getY() + y;
                if (boardY >= 0) {
                    drawCell(boardX, boardY, color, boardOffsetX_, boardOffsetY_);
                }
            }
        }
    }
}

void Renderer::drawNextPiece(const Tetromino& piece) {
    int sidebarX = boardOffsetX_ + Board::WIDTH * CELL_SIZE + PADDING;
    int nextPieceY = PADDING + 30;

    // Draw "NEXT" label
    drawLabel("NEXT", sidebarX, PADDING + 5);

    // Draw next piece preview box
    SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
    SDL_Rect previewRect = {sidebarX, nextPieceY, 4 * CELL_SIZE, 4 * CELL_SIZE};
    SDL_RenderFillRect(renderer_, &previewRect);

    // Draw border
    SDL_SetRenderDrawColor(renderer_, 80, 80, 80, 255);
    SDL_RenderDrawRect(renderer_, &previewRect);

    // Draw the next piece
    const auto& shape = piece.getShape();
    Color color = piece.getColor();

    for (int y = 0; y < Tetromino::SIZE; y++) {
        for (int x = 0; x < Tetromino::SIZE; x++) {
            if (shape[y][x]) {
                drawCell(x, y, color, sidebarX, nextPieceY);
            }
        }
    }
}

void Renderer::drawStats(int score, int level, int lines, int timeSeconds) {
    int sidebarX = boardOffsetX_ + Board::WIDTH * CELL_SIZE + PADDING;
    int startY = PADDING + 160;
    int rowHeight = 45;
    int scale = 2;

    // TIME
    drawLabel("TIME", sidebarX, startY);
    drawTime(timeSeconds, sidebarX, startY + 18);

    // SCORE
    drawLabel("SCORE", sidebarX, startY + rowHeight);
    SDL_SetRenderDrawColor(renderer_, 255, 255, 0, 255); // Yellow for score
    drawNumber(score, sidebarX, startY + rowHeight + 18, scale, 1);

    // LEVEL
    drawLabel("LEVEL", sidebarX, startY + rowHeight * 2);
    SDL_SetRenderDrawColor(renderer_, 0, 255, 255, 255); // Cyan for level
    drawNumber(level, sidebarX, startY + rowHeight * 2 + 18, scale, 1);

    // LINES
    drawLabel("LINES", sidebarX, startY + rowHeight * 3);
    SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255); // Green for lines
    drawNumber(lines, sidebarX, startY + rowHeight * 3 + 18, scale, 1);
}

void Renderer::drawGameOver() {
    // Draw semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 180);

    SDL_Rect overlay = {
        boardOffsetX_,
        boardOffsetY_,
        Board::WIDTH * CELL_SIZE,
        Board::HEIGHT * CELL_SIZE
    };
    SDL_RenderFillRect(renderer_, &overlay);

    // Draw "GAME OVER" text
    int centerX = boardOffsetX_ + (Board::WIDTH * CELL_SIZE) / 2;
    int centerY = boardOffsetY_ + (Board::HEIGHT * CELL_SIZE) / 2;

    SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);

    // "GAME" - 4 letters, each 12 pixels wide (scale 2)
    int scale = 3;
    int charWidth = 6 * scale;
    int gameWidth = 4 * charWidth;
    int overWidth = 4 * charWidth;

    int gameX = centerX - gameWidth / 2;
    int overX = centerX - overWidth / 2;

    // Draw GAME
    const char* game = "GAME";
    for (int i = 0; game[i] != '\0'; i++) {
        int idx = game[i] - 'A';
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (LETTER_FONT[idx][row] & (1 << (4 - col))) {
                    SDL_Rect pixel = {
                        gameX + i * charWidth + col * scale,
                        centerY - 30 + row * scale,
                        scale,
                        scale
                    };
                    SDL_RenderFillRect(renderer_, &pixel);
                }
            }
        }
    }

    // Draw OVER
    const char* over = "OVER";
    for (int i = 0; over[i] != '\0'; i++) {
        int idx = over[i] - 'A';
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (LETTER_FONT[idx][row] & (1 << (4 - col))) {
                    SDL_Rect pixel = {
                        overX + i * charWidth + col * scale,
                        centerY + 5 + row * scale,
                        scale,
                        scale
                    };
                    SDL_RenderFillRect(renderer_, &pixel);
                }
            }
        }
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

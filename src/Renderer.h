#pragma once

#include "Board.h"
#include "Tetromino.h"
#include <SDL.h>
#include <string>

class Renderer {
public:
    static constexpr int CELL_SIZE = 30;
    static constexpr int PADDING = 20;
    static constexpr int SIDEBAR_WIDTH = 150;

    Renderer();
    ~Renderer();

    bool init();
    void shutdown();

    void clear();
    void present();

    void drawBoard(const Board& board);
    void drawPiece(const Tetromino& piece);
    void drawNextPiece(const Tetromino& piece);
    void drawStats(int score, int level, int lines, int timeSeconds);
    void drawGameOver();

private:
    void drawCell(int x, int y, Color color, int offsetX = 0, int offsetY = 0);
    void drawDigit(int digit, int x, int y, int scale = 2);
    void drawNumber(int number, int x, int y, int scale = 2, int minDigits = 1);
    void drawLabel(const char* label, int x, int y);
    void drawTime(int totalSeconds, int x, int y);

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    int boardOffsetX_;
    int boardOffsetY_;
};

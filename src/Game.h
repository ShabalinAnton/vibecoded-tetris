#pragma once

#include "Board.h"
#include "Tetromino.h"
#include "Renderer.h"
#include "Sound.h"
#include "Music.h"
#include <memory>
#include <random>

class Game {
public:
    Game();

    bool init();
    void run();
    void shutdown();

private:
    void handleInput();
    void update();
    void render();

    void spawnNewPiece();
    void lockPiece();
    bool tryMove(int dx, int dy);
    bool tryRotate();
    void hardDrop();

    int calculateScore(int linesCleared);

    Board board_;
    Renderer renderer_;
    Sound sound_;
    Music music_;

    std::unique_ptr<Tetromino> currentPiece_;
    std::unique_ptr<Tetromino> nextPiece_;

    std::mt19937 rng_;

    bool running_ = false;
    bool gameOver_ = false;

    int score_ = 0;
    int level_ = 1;
    int totalLines_ = 0;

    Uint32 lastDropTime_ = 0;
    Uint32 dropInterval_ = 500; // milliseconds
    Uint32 gameStartTime_ = 0;

    static constexpr int LINES_PER_LEVEL = 10;
    static constexpr Uint32 MIN_DROP_INTERVAL = 50;
};

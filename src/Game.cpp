#include "Game.h"
#include <chrono>

Game::Game() : rng_(std::random_device{}()) {}

bool Game::init() {
    if (!renderer_.init()) {
        return false;
    }

    sound_.init(); // Audio is optional, continue even if it fails
    music_.init();
    music_.play();

    spawnNewPiece();
    spawnNewPiece(); // First call sets next, second sets current and new next

    lastDropTime_ = SDL_GetTicks();
    gameStartTime_ = SDL_GetTicks();
    running_ = true;

    return true;
}

void Game::run() {
    while (running_) {
        handleInput();

        if (!gameOver_) {
            update();
        }

        render();

        SDL_Delay(16); // ~60 FPS
    }
}

void Game::shutdown() {
    music_.shutdown();
    sound_.shutdown();
    renderer_.shutdown();
}

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running_ = false;
        }

        if (event.type == SDL_KEYDOWN) {
            if (gameOver_) {
                // Press any key to restart
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    board_.clear();
                    score_ = 0;
                    level_ = 1;
                    totalLines_ = 0;
                    dropInterval_ = 500;
                    gameOver_ = false;
                    gameStartTime_ = SDL_GetTicks();
                    currentPiece_.reset();
                    nextPiece_.reset();
                    spawnNewPiece();
                    spawnNewPiece();
                    music_.play();
                }
                continue;
            }

            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    if (tryMove(-1, 0)) {
                        sound_.play(SoundEffect::Move);
                    }
                    break;
                case SDLK_RIGHT:
                    if (tryMove(1, 0)) {
                        sound_.play(SoundEffect::Move);
                    }
                    break;
                case SDLK_DOWN:
                    if (tryMove(0, 1)) {
                        score_ += 1; // Soft drop bonus
                        sound_.play(SoundEffect::Move);
                    }
                    break;
                case SDLK_UP:
                    if (tryRotate()) {
                        sound_.play(SoundEffect::Rotate);
                    }
                    break;
                case SDLK_SPACE:
                    hardDrop();
                    break;
                case SDLK_ESCAPE:
                    running_ = false;
                    break;
            }
        }
    }
}

void Game::update() {
    Uint32 currentTime = SDL_GetTicks();

    if (currentTime - lastDropTime_ >= dropInterval_) {
        if (!tryMove(0, 1)) {
            lockPiece();
        }
        lastDropTime_ = currentTime;
    }
}

void Game::render() {
    renderer_.clear();
    renderer_.drawBoard(board_);

    if (currentPiece_) {
        renderer_.drawPiece(*currentPiece_);
    }

    if (nextPiece_) {
        renderer_.drawNextPiece(*nextPiece_);
    }

    // Calculate elapsed time in seconds
    int elapsedSeconds = (SDL_GetTicks() - gameStartTime_) / 1000;
    renderer_.drawStats(score_, level_, totalLines_, elapsedSeconds);

    if (gameOver_) {
        renderer_.drawGameOver();
    }

    renderer_.present();
}

void Game::spawnNewPiece() {
    if (nextPiece_) {
        currentPiece_ = std::move(nextPiece_);
    }

    // Generate random piece type
    std::uniform_int_distribution<int> dist(0, static_cast<int>(TetrominoType::Count) - 1);
    auto type = static_cast<TetrominoType>(dist(rng_));

    nextPiece_ = std::make_unique<Tetromino>(type);

    if (currentPiece_) {
        // Position piece at top center of board
        currentPiece_->setPosition(Board::WIDTH / 2 - 2, 0);

        // Check if spawn position is valid (game over if not)
        if (!board_.isValidPosition(*currentPiece_)) {
            gameOver_ = true;
            music_.stop();
            sound_.play(SoundEffect::GameOver);
        }
    }
}

void Game::lockPiece() {
    if (!currentPiece_) return;

    board_.placePiece(*currentPiece_);

    int linesCleared = board_.clearLines();
    if (linesCleared > 0) {
        score_ += calculateScore(linesCleared);
        totalLines_ += linesCleared;

        // Play appropriate sound for lines cleared
        if (linesCleared == 4) {
            sound_.play(SoundEffect::Tetris);
        } else {
            sound_.play(SoundEffect::LineClear);
        }

        // Level up
        int newLevel = totalLines_ / LINES_PER_LEVEL + 1;
        if (newLevel > level_) {
            level_ = newLevel;
            dropInterval_ = std::max(MIN_DROP_INTERVAL, static_cast<Uint32>(500 - (level_ - 1) * 50));
            sound_.play(SoundEffect::LevelUp);
        }
    }

    spawnNewPiece();
}

bool Game::tryMove(int dx, int dy) {
    if (!currentPiece_) return false;

    currentPiece_->move(dx, dy);

    if (!board_.isValidPosition(*currentPiece_)) {
        currentPiece_->move(-dx, -dy);
        return false;
    }

    return true;
}

bool Game::tryRotate() {
    if (!currentPiece_) return false;

    currentPiece_->rotateClockwise();

    if (!board_.isValidPosition(*currentPiece_)) {
        // Try wall kicks
        static const int kicks[][2] = {{-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {0, -1}};

        for (const auto& kick : kicks) {
            currentPiece_->move(kick[0], kick[1]);
            if (board_.isValidPosition(*currentPiece_)) {
                return true;
            }
            currentPiece_->move(-kick[0], -kick[1]);
        }

        // No valid position found, revert rotation
        currentPiece_->rotateCounterClockwise();
        return false;
    }

    return true;
}

void Game::hardDrop() {
    if (!currentPiece_) return;

    int dropDistance = 0;
    while (tryMove(0, 1)) {
        dropDistance++;
    }

    score_ += dropDistance * 2; // Hard drop bonus
    sound_.play(SoundEffect::Drop);
    lockPiece();
}

int Game::calculateScore(int linesCleared) {
    // Classic Tetris scoring
    static const int scoreTable[] = {0, 100, 300, 500, 800};
    return scoreTable[linesCleared] * level_;
}

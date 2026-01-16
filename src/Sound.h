#pragma once

#include <SDL.h>
#include <cmath>
#include <vector>
#include <queue>
#include <mutex>

enum class SoundEffect {
    Move,
    Rotate,
    Drop,
    LineClear,
    Tetris,     // 4 lines at once
    LevelUp,
    GameOver
};

class Sound {
public:
    Sound();
    ~Sound();

    bool init();
    void shutdown();

    void play(SoundEffect effect);

private:
    static void audioCallback(void* userdata, Uint8* stream, int len);
    void generateTone(float frequency, float duration, float volume = 0.3f);
    void generateSweep(float startFreq, float endFreq, float duration, float volume = 0.3f);

    SDL_AudioDeviceID audioDevice_ = 0;
    SDL_AudioSpec audioSpec_;

    std::vector<float> audioBuffer_;
    size_t bufferPosition_ = 0;
    std::mutex bufferMutex_;

    static constexpr int SAMPLE_RATE = 44100;
};

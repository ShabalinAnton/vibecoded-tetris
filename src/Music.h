#pragma once

#include <SDL.h>
#include <vector>
#include <array>
#include <atomic>

class Music {
public:
    Music();
    ~Music();

    bool init();
    void shutdown();

    void play();
    void stop();
    void setVolume(float volume);

private:
    static void audioCallback(void* userdata, Uint8* stream, int len);

    float generateSample();
    float generateKick(float t);
    float generateSnare(float t);
    float generateHiHat(float t);
    float generateBass(float t);
    float generateLead(float t);
    float generateArpeggio(float t);
    float generatePad(float t);

    float noteToFreq(int note);
    float saw(float phase);
    float square(float phase);
    float noise();

    SDL_AudioDeviceID audioDevice_ = 0;
    SDL_AudioSpec audioSpec_;

    std::atomic<bool> playing_{false};
    float volume_ = 0.5f;

    double sampleIndex_ = 0.0;

    // Timing
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr float BPM = 128.0f;
    static constexpr float BEAT_DURATION = 60.0f / BPM;
    static constexpr float BAR_DURATION = BEAT_DURATION * 4;

    // Musical elements
    static constexpr int PATTERN_LENGTH = 16; // 16th notes per bar

    // Bass pattern (MIDI notes, 0 = rest)
    std::array<int, PATTERN_LENGTH> bassPattern_;

    // Arpeggio notes
    std::array<int, 8> arpNotes_;

    // Current musical state
    int currentBar_ = 0;
    float phaseKick_ = 0;
    float phaseSnare_ = 0;
    float phaseBass_ = 0;
    float phaseLead_ = 0;
    float phaseArp_ = 0;
    float phasePad_ = 0;
};

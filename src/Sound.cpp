#include "Sound.h"
#include <algorithm>
#include <cstring>

Sound::Sound() {}

Sound::~Sound() {
    shutdown();
}

bool Sound::init() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        return false;
    }

    SDL_AudioSpec desired;
    SDL_memset(&desired, 0, sizeof(desired));
    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples = 1024;
    desired.callback = audioCallback;
    desired.userdata = this;

    audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &audioSpec_, 0);
    if (audioDevice_ == 0) {
        return false;
    }

    SDL_PauseAudioDevice(audioDevice_, 0); // Start audio
    return true;
}

void Sound::shutdown() {
    if (audioDevice_ != 0) {
        SDL_CloseAudioDevice(audioDevice_);
        audioDevice_ = 0;
    }
}

void Sound::audioCallback(void* userdata, Uint8* stream, int len) {
    Sound* sound = static_cast<Sound*>(userdata);
    float* floatStream = reinterpret_cast<float*>(stream);
    int samples = len / sizeof(float);

    std::lock_guard<std::mutex> lock(sound->bufferMutex_);

    for (int i = 0; i < samples; i++) {
        if (sound->bufferPosition_ < sound->audioBuffer_.size()) {
            floatStream[i] = sound->audioBuffer_[sound->bufferPosition_++];
        } else {
            floatStream[i] = 0.0f;
        }
    }

    // Clear consumed samples
    if (sound->bufferPosition_ >= sound->audioBuffer_.size()) {
        sound->audioBuffer_.clear();
        sound->bufferPosition_ = 0;
    }
}

void Sound::generateTone(float frequency, float duration, float volume) {
    int samples = static_cast<int>(SAMPLE_RATE * duration);

    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float envelope = 1.0f;

        // Apply envelope (attack and decay)
        float attackTime = 0.01f;
        float decayStart = duration - 0.05f;

        if (t < attackTime) {
            envelope = t / attackTime;
        } else if (t > decayStart) {
            envelope = (duration - t) / (duration - decayStart);
        }

        float sample = std::sin(2.0f * M_PI * frequency * t) * volume * envelope;
        audioBuffer_.push_back(sample);
    }
}

void Sound::generateSweep(float startFreq, float endFreq, float duration, float volume) {
    int samples = static_cast<int>(SAMPLE_RATE * duration);

    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // Interpolate frequency
        float frequency = startFreq + (endFreq - startFreq) * progress;

        // Apply envelope
        float envelope = 1.0f;
        float attackTime = 0.01f;
        float decayStart = duration - 0.05f;

        if (t < attackTime) {
            envelope = t / attackTime;
        } else if (t > decayStart) {
            envelope = (duration - t) / (duration - decayStart);
        }

        float sample = std::sin(2.0f * M_PI * frequency * t) * volume * envelope;
        audioBuffer_.push_back(sample);
    }
}

void Sound::play(SoundEffect effect) {
    std::lock_guard<std::mutex> lock(bufferMutex_);

    // Clear any existing audio for immediate response
    audioBuffer_.clear();
    bufferPosition_ = 0;

    switch (effect) {
        case SoundEffect::Move:
            // Short low click
            generateTone(200.0f, 0.05f, 0.2f);
            break;

        case SoundEffect::Rotate:
            // Quick ascending tone
            generateSweep(300.0f, 500.0f, 0.08f, 0.25f);
            break;

        case SoundEffect::Drop:
            // Thud sound - low frequency
            generateTone(100.0f, 0.1f, 0.4f);
            generateTone(80.0f, 0.1f, 0.3f);
            break;

        case SoundEffect::LineClear:
            // Pleasant chime
            generateTone(523.25f, 0.1f, 0.3f);  // C5
            generateTone(659.25f, 0.1f, 0.3f);  // E5
            generateTone(783.99f, 0.15f, 0.3f); // G5
            break;

        case SoundEffect::Tetris:
            // Triumphant fanfare for 4 lines
            generateTone(523.25f, 0.1f, 0.35f);  // C5
            generateTone(659.25f, 0.1f, 0.35f);  // E5
            generateTone(783.99f, 0.1f, 0.35f);  // G5
            generateTone(1046.50f, 0.2f, 0.4f);  // C6
            break;

        case SoundEffect::LevelUp:
            // Ascending arpeggio
            generateTone(261.63f, 0.1f, 0.3f);  // C4
            generateTone(329.63f, 0.1f, 0.3f);  // E4
            generateTone(392.00f, 0.1f, 0.3f);  // G4
            generateTone(523.25f, 0.2f, 0.35f); // C5
            break;

        case SoundEffect::GameOver:
            // Descending sad tones
            generateTone(400.0f, 0.2f, 0.3f);
            generateTone(300.0f, 0.2f, 0.3f);
            generateTone(200.0f, 0.3f, 0.25f);
            generateTone(150.0f, 0.4f, 0.2f);
            break;
    }
}

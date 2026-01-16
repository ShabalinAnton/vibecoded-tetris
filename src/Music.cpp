#include "Music.h"
#include <cmath>
#include <random>

Music::Music() {
    // Initialize bass pattern (E minor pentatonic style)
    // Pattern: E2, rest, E2, rest, G2, rest, E2, A2, rest, E2, rest, rest, G2, rest, E2, rest
    bassPattern_ = {40, 0, 40, 0, 43, 0, 40, 45, 0, 40, 0, 0, 43, 0, 40, 0};

    // Arpeggio notes (E minor chord with extensions)
    arpNotes_ = {52, 55, 59, 62, 64, 67, 71, 74}; // E4, G4, B4, D5, E5, G5, B5, D6
}

Music::~Music() {
    shutdown();
}

bool Music::init() {
    SDL_AudioSpec desired;
    SDL_memset(&desired, 0, sizeof(desired));
    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples = 2048;
    desired.callback = audioCallback;
    desired.userdata = this;

    audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &audioSpec_, 0);
    if (audioDevice_ == 0) {
        return false;
    }

    return true;
}

void Music::shutdown() {
    stop();
    if (audioDevice_ != 0) {
        SDL_CloseAudioDevice(audioDevice_);
        audioDevice_ = 0;
    }
}

void Music::play() {
    if (audioDevice_ != 0) {
        playing_ = true;
        SDL_PauseAudioDevice(audioDevice_, 0);
    }
}

void Music::stop() {
    playing_ = false;
    if (audioDevice_ != 0) {
        SDL_PauseAudioDevice(audioDevice_, 1);
    }
}

void Music::setVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
}

void Music::audioCallback(void* userdata, Uint8* stream, int len) {
    Music* music = static_cast<Music*>(userdata);
    float* floatStream = reinterpret_cast<float*>(stream);
    int samples = len / sizeof(float);

    for (int i = 0; i < samples; i++) {
        if (music->playing_) {
            floatStream[i] = music->generateSample() * music->volume_;
        } else {
            floatStream[i] = 0.0f;
        }
    }
}

float Music::generateSample() {
    double t = sampleIndex_ / SAMPLE_RATE;
    sampleIndex_++;

    // Calculate position in the music
    float barPosition = std::fmod(t, BAR_DURATION) / BAR_DURATION;
    float beatPosition = std::fmod(t, BEAT_DURATION) / BEAT_DURATION;
    int step = static_cast<int>(barPosition * PATTERN_LENGTH) % PATTERN_LENGTH;
    float stepPosition = std::fmod(barPosition * PATTERN_LENGTH, 1.0f);

    currentBar_ = static_cast<int>(t / BAR_DURATION) % 8;

    float sample = 0.0f;

    // Drums
    sample += generateKick(beatPosition) * 0.5f;
    sample += generateSnare(beatPosition) * 0.3f;
    sample += generateHiHat(stepPosition) * 0.15f;

    // Bass (comes in on bar 2)
    if (currentBar_ >= 1) {
        sample += generateBass(t) * 0.35f;
    }

    // Arpeggio (comes in on bar 3)
    if (currentBar_ >= 2) {
        sample += generateArpeggio(t) * 0.2f;
    }

    // Lead melody (comes in on bar 5)
    if (currentBar_ >= 4) {
        sample += generateLead(t) * 0.25f;
    }

    // Pad for atmosphere
    sample += generatePad(t) * 0.1f;

    // Soft clip to avoid harsh distortion
    sample = std::tanh(sample);

    return sample;
}

float Music::generateKick(float beatPos) {
    if (beatPos > 0.15f) return 0.0f;

    float t = beatPos * BEAT_DURATION;
    float freq = 150.0f * std::exp(-t * 40.0f) + 45.0f;
    float env = std::exp(-t * 15.0f);

    phaseKick_ += freq / SAMPLE_RATE;
    if (phaseKick_ > 1.0f) phaseKick_ -= 1.0f;

    return std::sin(phaseKick_ * 2.0f * M_PI) * env;
}

float Music::generateSnare(float beatPos) {
    // Snare on beats 2 and 4
    float snarePos = std::fmod(beatPos + 0.5f, 1.0f);
    if (snarePos > 0.12f) return 0.0f;

    float t = snarePos * BEAT_DURATION;
    float env = std::exp(-t * 20.0f);

    // Mix of tone and noise
    float tone = std::sin(phaseSnare_ * 2.0f * M_PI * 200.0f) * 0.3f;
    phaseSnare_ += 1.0f / SAMPLE_RATE;

    float noiseVal = noise() * 0.7f;

    return (tone + noiseVal) * env;
}

float Music::generateHiHat(float stepPos) {
    if (stepPos > 0.3f) return 0.0f;

    float t = stepPos * (BEAT_DURATION / 4.0f);
    float env = std::exp(-t * 50.0f);

    return noise() * env;
}

float Music::generateBass(float t) {
    float barPos = std::fmod(t, BAR_DURATION) / BAR_DURATION;
    int step = static_cast<int>(barPos * PATTERN_LENGTH) % PATTERN_LENGTH;
    float stepPos = std::fmod(barPos * PATTERN_LENGTH, 1.0f);

    int note = bassPattern_[step];
    if (note == 0) return 0.0f;

    float freq = noteToFreq(note);

    // Envelope
    float env = std::exp(-stepPos * 8.0f);

    phaseBass_ += freq / SAMPLE_RATE;
    if (phaseBass_ > 1.0f) phaseBass_ -= 1.0f;

    // Mix of saw and square for thick bass
    float sawVal = saw(phaseBass_);
    float sqVal = square(phaseBass_);

    return (sawVal * 0.6f + sqVal * 0.4f) * env;
}

float Music::generateArpeggio(float t) {
    // Fast arpeggio - 16th notes
    float stepDuration = BEAT_DURATION / 4.0f;
    int arpStep = static_cast<int>(t / stepDuration) % 8;
    float stepPos = std::fmod(t / stepDuration, 1.0f);

    int note = arpNotes_[arpStep];
    float freq = noteToFreq(note);

    // Short plucky envelope
    float env = std::exp(-stepPos * 12.0f);

    phaseArp_ += freq / SAMPLE_RATE;
    if (phaseArp_ > 1.0f) phaseArp_ -= 1.0f;

    // Bright saw wave
    return saw(phaseArp_) * env * 0.7f;
}

float Music::generateLead(float t) {
    // Simple melody pattern (2 bars)
    float melodyDuration = BAR_DURATION * 2;
    float melodyPos = std::fmod(t, melodyDuration) / melodyDuration;

    // Melody notes (16 half-beats over 2 bars)
    static const int melody[] = {
        64, 64, 67, 67, 71, 71, 69, 67, // E5, E5, G5, G5, B5, B5, A5, G5
        64, 67, 71, 74, 72, 71, 69, 67  // E5, G5, B5, D6, C6, B5, A5, G5
    };

    int noteIndex = static_cast<int>(melodyPos * 16) % 16;
    float notePos = std::fmod(melodyPos * 16, 1.0f);

    int note = melody[noteIndex];
    float freq = noteToFreq(note);

    // Smooth envelope
    float attack = std::min(notePos * 20.0f, 1.0f);
    float decay = std::exp(-notePos * 3.0f);
    float env = attack * decay;

    phaseLead_ += freq / SAMPLE_RATE;
    if (phaseLead_ > 1.0f) phaseLead_ -= 1.0f;

    // Mix of square and saw for synth lead
    float sqVal = square(phaseLead_);
    float sawVal = saw(phaseLead_);

    // Add slight detuned oscillator for width
    float detune = 1.005f;
    static float phase2 = 0;
    phase2 += (freq * detune) / SAMPLE_RATE;
    if (phase2 > 1.0f) phase2 -= 1.0f;

    float osc2 = saw(phase2);

    return ((sqVal * 0.4f + sawVal * 0.3f + osc2 * 0.3f)) * env;
}

float Music::generatePad(float t) {
    // Slow evolving pad for atmosphere
    // E minor chord: E3, G3, B3
    static const int padNotes[] = {40, 43, 47};
    float sample = 0.0f;

    for (int i = 0; i < 3; i++) {
        float freq = noteToFreq(padNotes[i]);
        float phase = std::fmod(t * freq, 1.0f);

        // Slow LFO for movement
        float lfo = std::sin(t * 0.5f + i * 0.5f) * 0.3f + 0.7f;

        sample += std::sin(phase * 2.0f * M_PI) * lfo;
    }

    return sample / 3.0f;
}

float Music::noteToFreq(int note) {
    // MIDI note to frequency (A4 = 440Hz = note 69)
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

float Music::saw(float phase) {
    return 2.0f * phase - 1.0f;
}

float Music::square(float phase) {
    return phase < 0.5f ? 1.0f : -1.0f;
}

float Music::noise() {
    static std::mt19937 gen(42);
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(gen);
}

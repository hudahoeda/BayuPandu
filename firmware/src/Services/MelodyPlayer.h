#pragma once

#include "HAL/IAudio.h"
#include <cstdint>

// One note of a buzzer tune: MIDI note number, when it starts and how long it
// sounds, in ms from the start of the tune. The buzzer plays one note at a time.
struct Note {
    uint16_t startMs;
    uint16_t durationMs;
    uint8_t midi;
};

// Frequency of a MIDI note in Hz (69 = A4 = 440 Hz).
float midiToHz(uint8_t midi);

// Plays a list of notes on the buzzer without blocking: call update() from the
// main loop and each note starts on time. The notes must be sorted by start and
// must not overlap; the list is copied, so it can be built on the stack.
class MelodyPlayer {
public:
    static constexpr int kMaxNotes = 32;

    explicit MelodyPlayer(IAudio& audio) : audio(audio) {}

    void play(const Note* notes, int count, uint32_t nowMs);
    void update(uint32_t nowMs);
    void stop();
    bool isPlaying() const { return playing; }

private:
    IAudio& audio;
    Note notes[kMaxNotes];
    int count = 0;
    int next = 0;
    uint32_t startMs = 0;
    bool playing = false;
};

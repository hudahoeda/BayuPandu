#include "MelodyPlayer.h"
#include <cmath>

float midiToHz(uint8_t midi) {
    return 440.0f * std::pow(2.0f, (static_cast<int>(midi) - 69) / 12.0f);
}

void MelodyPlayer::play(const Note* source, int n, uint32_t nowMs) {
    count = n < kMaxNotes ? n : kMaxNotes;
    for (int i = 0; i < count; i++) notes[i] = source[i];
    next = 0;
    startMs = nowMs;
    playing = count > 0;
    update(nowMs);
}

void MelodyPlayer::update(uint32_t nowMs) {
    if (!playing) return;
    uint32_t elapsed = nowMs - startMs;
    while (next < count && notes[next].startMs <= elapsed) {
        const Note& note = notes[next++];
        // A slow loop can arrive after a short note has already ended; skip it
        // rather than play the tune out of time.
        if (elapsed < static_cast<uint32_t>(note.startMs) + note.durationMs) {
            uint32_t remaining = note.startMs + note.durationMs - elapsed;
            audio.tone(static_cast<unsigned int>(std::lround(midiToHz(note.midi))), remaining);
        }
    }
    if (next >= count) {
        const Note& last = notes[count - 1];
        if (elapsed >= static_cast<uint32_t>(last.startMs) + last.durationMs) playing = false;
    }
}

void MelodyPlayer::stop() {
    if (playing) audio.noTone();
    playing = false;
}

// Buzzer for the desktop simulator: a square-wave voice, like the piezo on the
// device, played through SDL audio. Also renders a note list to WAV and to a
// standard MIDI file, so the startup chime can be heard and edited off-device.
#pragma once

#include <SDL.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "HAL/IAudio.h"
#include "Services/MelodyPlayer.h"

// One monophonic square voice with 2 ms ramps at note start and end, so notes
// don't click. Duration 0 means "until noTone()". The square is band-limited
// (PolyBLEP): a naive one's harmonics above half the sample rate fold back as
// off-key undertones, which a real piezo doesn't make.
class SquareSynth {
public:
    explicit SquareSynth(int sampleRate) : rate(sampleRate) {}

    void start(float hz, unsigned long durationMs) {
        freq = hz;
        samplesLeft = durationMs ? static_cast<int64_t>(durationMs) * rate / 1000 : INT64_MAX;
    }
    void stop() { samplesLeft = 0; }

    void render(int16_t* out, int n) {
        const float ramp = 1.0f / (0.002f * rate);
        for (int i = 0; i < n; i++) {
            float target = samplesLeft > 0 ? 1.0f : 0.0f;
            gain += gain < target ? ramp : (gain > target ? -ramp : 0.0f);
            gain = gain < 0 ? 0 : (gain > 1 ? 1 : gain);
            if (samplesLeft > 0) samplesLeft--;
            const float dt = freq / rate;
            phase += dt;
            if (phase >= 1.0f) phase -= 1.0f;
            float falling = phase + 0.5f;
            if (falling >= 1.0f) falling -= 1.0f;
            float square = (phase < 0.5f ? 1.0f : -1.0f) + blep(phase, dt) - blep(falling, dt);
            out[i] = static_cast<int16_t>(square * gain * kLevel * 32767.0f);
        }
    }

private:
    // Smooths the step at phase 0 over one sample either side.
    static float blep(float t, float dt) {
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.0f;
        }
        if (t > 1.0f - dt) {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }
        return 0.0f;
    }

    static constexpr float kLevel = 0.18f;  // a square wave is loud; keep headroom
    int rate;
    float freq = 0, phase = 0, gain = 0;
    int64_t samplesLeft = 0;
};

class SimAudio : public IAudio {
public:
    static constexpr int kRate = 44100;

    // Opens the sound device; without one (headless runs, no audio) it stays silent.
    bool open() {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;
        SDL_AudioSpec want{}, have{};
        want.freq = kRate;
        want.format = AUDIO_S16SYS;
        want.channels = 1;
        want.samples = 512;
        want.callback = [](void* self, Uint8* stream, int len) {
            static_cast<SimAudio*>(self)->synth.render(reinterpret_cast<int16_t*>(stream), len / 2);
        };
        want.userdata = this;
        device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
        if (!device) return false;
        SDL_PauseAudioDevice(device, 0);
        return true;
    }
    ~SimAudio() {
        if (device) SDL_CloseAudioDevice(device);
    }

    bool initialize() override { return true; }
    void tone(unsigned int frequency, unsigned long duration = 0) override {
        if (!device || muted) return;
        SDL_LockAudioDevice(device);
        synth.start(static_cast<float>(frequency), duration);
        SDL_UnlockAudioDevice(device);
    }
    void noTone() override {
        if (!device) return;
        SDL_LockAudioDevice(device);
        synth.stop();
        SDL_UnlockAudioDevice(device);
    }
    void setVolume(uint8_t percentage) override { muted = percentage == 0; }
    void playAlert(AlertType) override {}

private:
    SquareSynth synth{kRate};
    SDL_AudioDeviceID device = 0;
    bool muted = false;
};

// --- Files ------------------------------------------------------------------------

inline void putLE(std::vector<uint8_t>& b, uint32_t v, int bytes) {
    for (int i = 0; i < bytes; i++) b.push_back(static_cast<uint8_t>(v >> (8 * i)));
}
inline void putBE(std::vector<uint8_t>& b, uint32_t v, int bytes) {
    for (int i = bytes - 1; i >= 0; i--) b.push_back(static_cast<uint8_t>(v >> (8 * i)));
}
inline bool writeFile(const std::string& path, const std::vector<uint8_t>& data) {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = std::fwrite(data.data(), 1, data.size(), f) == data.size();
    return std::fclose(f) == 0 && ok;
}

// The notes as the buzzer plays them: 16-bit mono WAV through the same synth.
inline bool writeWav(const std::string& path, const Note* notes, int count) {
    constexpr int rate = SimAudio::kRate;
    uint32_t endMs = 0;
    for (int i = 0; i < count; i++) endMs = std::max<uint32_t>(endMs, notes[i].startMs + notes[i].durationMs);
    int total = static_cast<int>((endMs + 100) * static_cast<int64_t>(rate) / 1000);
    std::vector<int16_t> pcm(total);
    SquareSynth synth(rate);
    int next = 0, pos = 0;
    while (pos < total) {
        int at = next < count ? static_cast<int>(notes[next].startMs * static_cast<int64_t>(rate) / 1000) : total;
        if (at <= pos) {
            synth.start(midiToHz(notes[next].midi), notes[next].durationMs);
            next++;
            continue;
        }
        int n = std::min(at, total) - pos;
        synth.render(pcm.data() + pos, n);
        pos += n;
    }
    std::vector<uint8_t> b = {'R', 'I', 'F', 'F'};
    putLE(b, 36 + total * 2, 4);
    for (char c : std::string("WAVEfmt ")) b.push_back(c);
    putLE(b, 16, 4); putLE(b, 1, 2); putLE(b, 1, 2); putLE(b, rate, 4);
    putLE(b, rate * 2, 4); putLE(b, 2, 2); putLE(b, 16, 2);
    for (char c : std::string("data")) b.push_back(c);
    putLE(b, total * 2, 4);
    for (int16_t s : pcm) putLE(b, static_cast<uint16_t>(s), 2);
    return writeFile(path, b);
}

// The notes as a type-0 standard MIDI file: 1000 ticks per beat at 60 bpm, so one
// tick is one millisecond. The instrument is General MIDI "Lead 1 (square)".
inline bool writeMidi(const std::string& path, const Note* notes, int count) {
    struct Event { uint32_t ms; uint8_t status, data1, data2; };
    std::vector<Event> events;
    for (int i = 0; i < count; i++) {
        events.push_back({notes[i].startMs, 0x90, notes[i].midi, 100});
        events.push_back({static_cast<uint32_t>(notes[i].startMs + notes[i].durationMs), 0x80, notes[i].midi, 0});
    }
    std::stable_sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.ms != b.ms ? a.ms < b.ms : a.status < b.status;  // note-off before a note-on at the same tick
    });
    std::vector<uint8_t> track;
    auto delta = [&](uint32_t v) {
        uint8_t buf[5];
        int n = 0;
        buf[n++] = v & 0x7F;
        while (v >>= 7) buf[n++] = 0x80 | (v & 0x7F);
        while (n) track.push_back(buf[--n]);
    };
    delta(0); track.insert(track.end(), {0xFF, 0x51, 0x03, 0x0F, 0x42, 0x40});  // tempo: 1,000,000 us/beat
    delta(0); track.insert(track.end(), {0xFF, 0x03, 0x0E});                    // track name
    for (char c : std::string("BayuPandu boot")) track.push_back(c);
    delta(0); track.insert(track.end(), {0xC0, 80});                            // program: square lead
    uint32_t last = 0;
    for (const Event& e : events) {
        delta(e.ms - last);
        last = e.ms;
        track.insert(track.end(), {e.status, e.data1, e.data2});
    }
    delta(0); track.insert(track.end(), {0xFF, 0x2F, 0x00});                    // end of track
    std::vector<uint8_t> b = {'M', 'T', 'h', 'd'};
    putBE(b, 6, 4); putBE(b, 0, 2); putBE(b, 1, 2); putBE(b, 1000, 2);
    for (char c : std::string("MTrk")) b.push_back(c);
    putBE(b, static_cast<uint32_t>(track.size()), 4);
    b.insert(b.end(), track.begin(), track.end());
    return writeFile(path, b);
}

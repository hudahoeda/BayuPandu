// BayuPandu desktop simulator.
//
// Runs the real firmware UI + services on the desktop: LVGL renders into an SDL
// window (scaled 240x320 ILI9341 framebuffer), keyboard keys stand in for the four
// buttons, and an IGC file is replayed through FlightManager's simulation mode.
//
//   bayupandu_sim [igc] [--speed N] [--scale N]
//   bayupandu_sim [igc] --shots DIR [--at SECONDS]   (headless: one PNG per screen)

#include <SDL.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "Arduino.h"
#include "HAL/IArduino.h"
#include "HAL/AudioImpl.h"
#include "HAL/BarometerImpl.h"
#include "HAL/GPSImpl.h"
#include "HAL/IMUImpl.h"
#include "HAL/PowerImpl.h"
#include "HAL/StorageImpl.h"
#include "Services/ConfigService.h"
#include "Services/FlightManager.h"
#include "Services/GPSService.h"
#include "Services/IMUService.h"
#include "Services/PowerService.h"
#include "Services/VariometerService.h"
#include "UI/LVGLInit.h"
#include "UI/UserInterface.h"
#include "config.h"
#include "png_writer.h"

static constexpr int kWidth = 240;
static constexpr int kHeight = 320;

// ---------------------------------------------------------------------------
// Simulated clock + button pins

static uint64_t g_simMillis = 0;
static bool g_pinPressed[64] = {};
// A quick key tap delivers KEYDOWN and KEYUP in the same frame, which the firmware
// would never see (it polls pins and debounces for 50 ms). Hold each press at least
// this long in simulated time before releasing it.
static constexpr uint64_t kMinPressMs = 150;
static uint64_t g_pressStart[64] = {};
static uint64_t g_releaseAt[64] = {};  // 0 = no pending release

uint32_t millis() { return static_cast<uint32_t>(g_simMillis); }
void delay(uint32_t ms) { g_simMillis += ms; }
void pinMode(uint8_t, uint8_t) {}
void digitalWrite(uint8_t, uint8_t) {}
int digitalRead(uint8_t pin) { return (pin < 64 && g_pinPressed[pin]) ? LOW : HIGH; }

SimSerial Serial;
void SimSerial::print(const char* s) { std::fputs(s, stdout); }
void SimSerial::println(const char* s) { std::puts(s); }

class SimArduino : public IArduino {
public:
    uint32_t millis() override { return ::millis(); }
    void delay(uint32_t ms) override { ::delay(ms); }
    void pinMode(uint8_t pin, uint8_t mode) override { ::pinMode(pin, mode); }
    void digitalWrite(uint8_t pin, uint8_t value) override { ::digitalWrite(pin, value); }
    int digitalRead(uint8_t pin) override { return ::digitalRead(pin); }
    void serialBegin(uint32_t) override {}
    void serialPrint(const char* text) override { Serial.print(text); }
    void serialPrintln(const char* text) override { Serial.println(text); }
    int serialAvailable() override { return 0; }
    int serialRead() override { return -1; }
    uint8_t getLedBuiltinPin() override { return PIN_LED_BUILTIN; }
};

// ---------------------------------------------------------------------------
// Framebuffer: LVGL flushes RGB565 areas here; SDL / PNG read from it.

static uint32_t g_framebuffer[kWidth * kHeight];  // ARGB8888

static void simFlush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* colors) {
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            lv_color_t c = *colors++;
            if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) continue;
            uint32_t argb = lv_color_to32(c);
            g_framebuffer[y * kWidth + x] = argb | 0xFF000000u;
        }
    }
    lv_disp_flush_ready(disp);
}

static bool savePng(const std::string& path) {
    std::vector<uint8_t> rgb(kWidth * kHeight * 3);
    for (int i = 0; i < kWidth * kHeight; i++) {
        rgb[i * 3 + 0] = (g_framebuffer[i] >> 16) & 0xFF;
        rgb[i * 3 + 1] = (g_framebuffer[i] >> 8) & 0xFF;
        rgb[i * 3 + 2] = g_framebuffer[i] & 0xFF;
    }
    return writePngRgb(path, kWidth, kHeight, rgb.data());
}

// ---------------------------------------------------------------------------
// Firmware wiring (mirrors setup() in src/main.cpp)

struct Firmware {
    SimArduino arduino;
    BarometerImpl barometer;
    AudioImpl audio;
    GPSImpl gps;
    IMUImpl imu;
    PowerImpl power;
    StorageImpl storage;

    ConfigService* configService = nullptr;
    VariometerService* variometerService = nullptr;
    GPSService* gpsService = nullptr;
    IMUService* imuService = nullptr;
    PowerService* powerService = nullptr;
    FlightManager* flightManager = nullptr;
    UserInterface* userInterface = nullptr;

    void setup() {
        configService = new ConfigService(storage);
        variometerService = new VariometerService(barometer, audio, arduino);
        gpsService = new GPSService(gps);
        imuService = new IMUService(imu);
        powerService = new PowerService(power, audio, *configService, arduino);

        LVGLInit::initialize(arduino);
        // Redirect the ILI9341 flush to our framebuffer.
        lv_disp_get_default()->driver->flush_cb = simFlush;

        flightManager = new FlightManager(*variometerService, *gpsService, *imuService,
                                          *powerService, *configService, storage, arduino);
        userInterface = new UserInterface(*flightManager, *configService, arduino,
                                          *LVGLInit::inputManager);
        flightManager->setUserInterface(userInterface);
        LVGLInit::setUserInterface(userInterface);
        flightManager->initialize();
        userInterface->initialize();
    }

    std::string replayPath;  // set when replaying; the replay restarts when it runs out

    void loop() {
        LVGLInit::handler();
        flightManager->update();
        if (!replayPath.empty() && !flightManager->getFusedFlightData().isValid) {
            flightManager->enableSimulation(replayPath);
        }
    }
};

// Parses "SECONDS", "MM:SS" or "H:MM:SS" into seconds; -1 when malformed.
static long parseClock(const std::string& text) {
    long total = 0, part = 0;
    bool digits = false;
    for (char c : text) {
        if (c >= '0' && c <= '9') { part = part * 10 + (c - '0'); digits = true; }
        else if (c == ':' && digits) { total = total * 60 + part; part = 0; digits = false; }
        else return -1;
    }
    return digits ? total * 60 + part : -1;
}

// Writes the IGC headers plus the B-records from `fromSec` (after the first fix) for
// `lengthSec` seconds to a temp file, so a short stretch of a long flight can be
// replayed and looped. Returns the path, or "" when the window holds no fixes.
static std::string sliceIgc(const std::string& path, long fromSec, long lengthSec) {
    std::ifstream in(path);
    if (!in) return "";
    std::filesystem::path out = std::filesystem::temp_directory_path() / "bayupandu-replay-slice.igc";
    std::ofstream os(out);
    std::string line;
    long firstFix = -1;
    int kept = 0;
    while (std::getline(in, line)) {
        if (line.size() < 7 || line[0] != 'B') {
            if (kept == 0) os << line << "\n";  // headers before the first fix
            continue;
        }
        long t = std::stol(line.substr(1, 2)) * 3600 + std::stol(line.substr(3, 2)) * 60 +
                 std::stol(line.substr(5, 2));
        if (firstFix < 0) firstFix = t;
        long since = t - firstFix;
        if (since >= fromSec && (lengthSec <= 0 || since < fromSec + lengthSec)) {
            os << line << "\n";
            kept++;
        }
    }
    return kept > 1 ? out.string() : "";
}

// Advance simulated time in fixed steps so headless runs are deterministic.
static void runFor(Firmware& fw, uint64_t ms, uint32_t stepMs = 20) {
    for (uint64_t t = 0; t < ms; t += stepMs) {
        g_simMillis += stepMs;
        fw.loop();
    }
}

static int runHeadless(Firmware& fw, const std::string& dir, double atSeconds) {
    runFor(fw, static_cast<uint64_t>(atSeconds * 1000.0));

    auto capture = [&](const char* name) {
        runFor(fw, 400);  // let the UI's refresh interval elapse and redraw
        lv_obj_invalidate(lv_scr_act());
        lv_refr_now(nullptr);
        std::string path = dir + "/" + name + ".png";
        if (!savePng(path)) {
            std::fprintf(stderr, "failed to write %s\n", path.c_str());
            return false;
        }
        std::printf("wrote %s\n", path.c_str());
        return true;
    };
    // Buttons as the firmware numbers them: 0 UP, 1 DOWN, 2 ENTER, 3 CANCEL.
    auto press = [&](uint8_t button) {
        fw.userInterface->handleButtonAction(ButtonAction::SHORT_PRESS, button);
    };

    const struct { DisplayScreen screen; const char* name; } pages[] = {
        {DisplayScreen::MAIN_FLIGHT, "1-flight"},
        {DisplayScreen::THERMAL, "2-thermal"},
        {DisplayScreen::NAVIGATION, "3-navigation"},
        {DisplayScreen::STATUS, "4-status"},
        {DisplayScreen::SETTINGS, "5-settings"},
        {DisplayScreen::ERROR, "7-error"},
    };
    for (const auto& page : pages) {
        fw.userInterface->setScreen(page.screen);
        if (!capture(page.name)) return 1;
        if (page.screen == DisplayScreen::SETTINGS) {
            press(2);  // ENTER: edit
            press(1);  // DOWN: focus the second setting
            if (!capture("6-settings-editing")) return 1;
            press(3);  // CANCEL: done
        }
    }
    return 0;
}

static int runInteractive(Firmware& fw, double speed, int scale) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("BayuPandu simulator", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, kWidth * scale,
                                          kHeight * scale, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, kWidth, kHeight);

    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::puts("Keys: Up/Down = previous/next page, Enter = edit (Settings),");
    std::puts("      Esc/Backspace = CANCEL (back to Flight), S = screenshot, Q = quit");

    auto keyToPin = [](SDL_Keycode key) -> int {
        switch (key) {
            case SDLK_UP: return PIN_UP;
            case SDLK_DOWN: return PIN_DOWN;
            case SDLK_RETURN: return PIN_ENTER;
            case SDLK_ESCAPE:
            case SDLK_BACKSPACE: return PIN_CANCEL;
            default: return -1;
        }
    };

    auto last = std::chrono::steady_clock::now();
    double carryMs = 0.0;
    int shotCount = 0;
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                bool down = e.type == SDL_KEYDOWN;
                if (down && e.key.keysym.sym == SDLK_q) running = false;
                if (down && !e.key.repeat && e.key.keysym.sym == SDLK_s) {
                    std::string path = "bayupandu-" + std::to_string(++shotCount) + ".png";
                    if (savePng(path)) std::printf("wrote %s\n", path.c_str());
                }
                int pin = keyToPin(e.key.keysym.sym);
                if (pin >= 0 && down && !e.key.repeat) {
                    g_pinPressed[pin] = true;
                    g_pressStart[pin] = g_simMillis;
                    g_releaseAt[pin] = 0;
                } else if (pin >= 0 && !down) {
                    g_releaseAt[pin] = std::max(g_simMillis, g_pressStart[pin] + kMinPressMs);
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        carryMs += std::chrono::duration<double, std::milli>(now - last).count() * speed;
        last = now;
        // Step in <=20 ms chunks so button debounce and timers still see sane deltas.
        while (carryMs >= 1.0) {
            uint32_t step = carryMs > 20.0 ? 20 : static_cast<uint32_t>(carryMs);
            g_simMillis += step;
            carryMs -= step;
            for (int pin = 0; pin < 64; pin++) {
                if (g_releaseAt[pin] && g_simMillis >= g_releaseAt[pin]) {
                    g_pinPressed[pin] = false;
                    g_releaseAt[pin] = 0;
                }
            }
            fw.loop();
        }

        SDL_UpdateTexture(texture, nullptr, g_framebuffer, kWidth * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int main(int argc, char** argv) {
    std::string igcPath;
    std::string shotsDir;
    double atSeconds = 60.0;
    std::string fromText;
    long lengthSec = 180;
    double speed = 1.0;
    int scale = 2;
    uint8_t language = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        auto next = [&]() -> const char* { return i + 1 < argc ? argv[++i] : ""; };
        if (arg == "--shots") shotsDir = next();
        else if (arg == "--at") atSeconds = std::atof(next());
        else if (arg == "--speed") speed = std::atof(next());
        else if (arg == "--scale") scale = std::atoi(next());
        else if (arg == "--lang") language = std::string(next()) == "id" ? 1 : 0;
        else if (arg == "--from") fromText = next();
        else if (arg == "--length") lengthSec = parseClock(next());
        else if (arg == "-h" || arg == "--help") {
            std::puts("usage: bayupandu_sim [file.igc] [--from TIME] [--length TIME] [--speed N] [--scale N] [--lang en|id]\n"
                      "       bayupandu_sim [file.igc] [--from TIME] [--length TIME] --shots DIR [--at SECONDS]\n"
                      "\n"
                      "Replays --length (default 3:00, 0 = whole flight) starting --from after the first\n"
                      "fix, looping. TIME is SECONDS, MM:SS or H:MM:SS. With no file, plays a thermal\n"
                      "and glide from the bundled example flight.");
            return 0;
        } else igcPath = arg;
    }

    Firmware fw;
    fw.setup();
    fw.configService->getConfig().language = language;

    lv_mem_monitor_t mem;
    lv_mem_monitor(&mem);
    std::printf("LVGL heap: %u of %u bytes used (%u%%), largest free block %u\n",
                (unsigned)(mem.total_size - mem.free_size), (unsigned)mem.total_size,
                (unsigned)mem.used_pct, (unsigned)mem.free_biggest_size);
    if (igcPath.empty()) {
        igcPath = BAYUPANDU_EXAMPLE_IGC;
        if (fromText.empty()) fromText = "1:49:30";  // a strong thermal, then a glide
    }
    long fromSec = fromText.empty() ? 0 : parseClock(fromText);
    if (fromSec < 0 || lengthSec < 0) {
        std::fprintf(stderr, "bad --from/--length; use SECONDS, MM:SS or H:MM:SS\n");
        return 2;
    }
    std::string replay = sliceIgc(igcPath, fromSec, lengthSec);
    if (replay.empty()) {
        std::fprintf(stderr, "no fixes in %s for that --from/--length window\n", igcPath.c_str());
        return 2;
    }
    fw.flightManager->enableSimulation(replay);
    if (!fw.flightManager->isSimulationActive()) {
        std::fprintf(stderr, "could not load IGC file: %s\n", igcPath.c_str());
        return 2;
    }
    fw.replayPath = replay;
    std::printf("Replaying %s from +%ld s for %s, looping\n", igcPath.c_str(), fromSec,
                lengthSec > 0 ? (std::to_string(lengthSec) + " s").c_str() : "the whole flight");

    return shotsDir.empty() ? runInteractive(fw, speed, scale)
                            : runHeadless(fw, shotsDir, atSeconds);
}

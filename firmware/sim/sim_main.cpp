// BayuPandu desktop simulator.
//
// Runs the real firmware UI + services on the desktop: LVGL renders into an SDL
// window (scaled 240x320 ILI9341 framebuffer), keyboard keys stand in for the four
// buttons, and an IGC file is replayed through FlightManager's simulation mode.
//
//   bayupandu_sim [igc] [--speed N] [--scale N] [--no-gps] [--no-data] [--battery PCT]
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
#include "HAL/BarometerImpl.h"
#include "HAL/GPSImpl.h"
#include "HAL/IMUImpl.h"
#include "HAL/IPower.h"
#include "Services/ConfigService.h"
#include "Services/FlightManager.h"
#include "Services/GPSService.h"
#include "Services/IMUService.h"
#include "Services/PowerService.h"
#include "Services/VariometerService.h"
#include "UI/BootChime.h"
#include "UI/LVGLInit.h"
#include "UI/UserInterface.h"
#include "config.h"
#include "png_writer.h"
#include "sim_audio.h"
#include "sim_storage.h"

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

// Battery with a charge the simulator can set, so the low and critical battery
// warnings can be seen. PowerService maps voltage linearly from the critical
// level (0%) to 4.2 V (100%), so the voltage is derived from that same scale.
class SimPower : public IPower {
public:
    bool initialize() override { return true; }
    float getBatteryVoltage() override { return voltage; }
    uint8_t getBatteryPercentage() override { return percent; }
    bool isCharging() override { return false; }
    void enterLowPowerMode() override {}

    void setPercent(int pct, float criticalVolts) {
        percent = static_cast<uint8_t>(std::clamp(pct, 0, 100));
        // A hair above the exact step so integer truncation lands on `pct`.
        voltage = criticalVolts + (4.2f - criticalVolts) * (percent + 0.5f) / 100.0f;
    }
    int getPercent() const { return percent; }

private:
    uint8_t percent = 100;
    float voltage = 4.2f;
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
    explicit Firmware(const std::string& card) : storage(card) {}

    SimArduino arduino;
    BarometerImpl barometer;
    SimAudio audio;
    GPSImpl gps;
    IMUImpl imu;
    SimPower power;
    SimStorage storage;  // the SD card, a folder

    ConfigService* configService = nullptr;
    VariometerService* variometerService = nullptr;
    GPSService* gpsService = nullptr;
    IMUService* imuService = nullptr;
    PowerService* powerService = nullptr;
    FlightManager* flightManager = nullptr;
    UserInterface* userInterface = nullptr;

    SimulationService& sim() { return flightManager->getSimulationService(); }

    void setBattery(int pct) {
        power.setPercent(pct, configService->getConfig().criticalBatteryLevel);
    }

    void printFaults() {
        std::printf("faults: GPS %s, data %s, battery %d%%\n", sim().isGpsLost() ? "LOST" : "ok",
                    sim().isBaroLost() ? "LOST" : "ok", power.getPercent());
    }

    void setup() {
        storage.initialize();
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
        flightManager->getFlightLogger().setDevice("SIM", "BayuPandu simulator");
        userInterface->setAudio(&audio);
        LVGLInit::setUserInterface(userInterface);
        flightManager->initialize();
        userInterface->initialize();
    }

    std::string replayPath;  // set when replaying; the replay restarts when it runs out

    void loop() {
        LVGLInit::handler();
        flightManager->update();
        // Restart when the replay runs out (not merely when data is invalid, which
        // the --no-data fault makes it on purpose).
        if (!replayPath.empty() && !flightManager->getSimulationService().isActive()) {
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

static void printHeap(const char* when) {
    lv_mem_monitor_t mem;
    lv_mem_monitor(&mem);
    std::printf("LVGL heap %s: %u of %u bytes used, largest free block %u\n", when,
                (unsigned)(mem.total_size - mem.free_size), (unsigned)mem.total_size,
                (unsigned)mem.free_biggest_size);
}

static bool captureNow(const std::string& path) {
    lv_refr_now(nullptr);
    if (!savePng(path)) {
        std::fprintf(stderr, "failed to write %s\n", path.c_str());
        return false;
    }
    return true;
}

// Writes one PNG per 40 ms frame of the boot sequence, for previews and video.
static int runBootFrames(Firmware& fw, const std::string& dir) {
    char name[32];
    for (int frame = 0; frame * 40 <= 3600; frame++) {
        runFor(fw, 40);
        std::snprintf(name, sizeof(name), "/boot-%03d.png", frame);
        if (!captureNow(dir + name)) return 1;
    }
    std::printf("wrote %s/boot-*.png (25 fps)\n", dir.c_str());
    return 0;
}

// Writes the replay at 10 Hz as CSV: the log's own climb next to what the vario
// filter makes of it, for checking and tuning the filter against real flights.
static int runTrace(Firmware& fw, const std::string& path, long lengthSec) {
    std::ofstream out(path);
    if (!out) {
        std::fprintf(stderr, "failed to write %s\n", path.c_str());
        return 1;
    }
    out << "t_s,log_climb_ms,vario_climb_ms,replay_alt_m,vario_alt_m\n";
    out.setf(std::ios::fixed);
    out.precision(3);
    for (long tick = 0; tick < lengthSec * 10; tick++) {
        runFor(fw, 100);
        const FlightData& d = fw.flightManager->getFusedFlightData();
        SimulationService& sim = fw.sim();
        out << tick / 10.0 << ',' << sim.getVerticalSpeed() << ',' << d.verticalSpeed << ','
            << sim.getBarometricAltitude() << ',' << d.altitude << '\n';
    }
    std::printf("wrote %s\n", path.c_str());
    return 0;
}

static int runHeadless(Firmware& fw, const std::string& dir, double atSeconds) {
    // The boot sign with every check shown, during its hold.
    runFor(fw, 2100);
    printHeap("during boot");
    if (!captureNow(dir + "/0-boot.png")) return 1;
    std::printf("wrote %s/0-boot.png\n", dir.c_str());
    runFor(fw, static_cast<uint64_t>(std::max(0.0, atSeconds * 1000.0 - 2100.0)));
    printHeap("in flight");

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

    // Warning states, one fault at a time, on the Flight page where they matter most.
    // The footer shows the most urgent: critical battery, no data, no GPS, low battery.
    const struct { const char* name; bool gpsLost; bool baroLost; int battery; } faults[] = {
        {"8-warn-no-gps", true, false, 100},
        {"9-warn-low-battery", false, false, 15},
        {"10-warn-critical-battery", false, false, 3},
        {"11-warn-data-lost", false, true, 100},
    };
    fw.userInterface->setScreen(DisplayScreen::MAIN_FLIGHT);
    for (const auto& f : faults) {
        fw.sim().setGpsLost(f.gpsLost);
        fw.sim().setBaroLost(f.baroLost);
        fw.setBattery(f.battery);
        if (!capture(f.name)) return 1;
    }
    // GPS loss also blanks ground speed and the navigation figures.
    fw.sim().setBaroLost(false);
    fw.setBattery(100);
    fw.sim().setGpsLost(true);
    fw.userInterface->setScreen(DisplayScreen::NAVIGATION);
    if (!capture("12-navigation-no-gps")) return 1;
    return 0;
}

// Writes the startup chime as MIDI and WAV: every check passing, and with the
// barometer failing, from the same composition the firmware plays.
static int exportChime(const std::string& dir) {
    const struct { const char* name; bool passed[BootTiming::kChecks]; } variants[] = {
        {"boot-chime", {true, true, true, true}},
        {"boot-chime-fault", {false, true, true, true}},
    };
    for (const auto& v : variants) {
        Note notes[MelodyPlayer::kMaxNotes];
        int n = BootChime::compose(v.passed, notes, MelodyPlayer::kMaxNotes);
        std::string base = dir + "/" + v.name;
        if (!writeMidi(base + ".mid", notes, n) || !writeWav(base + ".wav", notes, n)) {
            std::fprintf(stderr, "failed to write %s.mid/.wav\n", base.c_str());
            return 1;
        }
        std::printf("wrote %s.mid and %s.wav (%d notes)\n", base.c_str(), base.c_str(), n);
    }
    return 0;
}

static int runInteractive(Firmware& fw, double speed, int scale, bool mute) {
    SDL_Init(SDL_INIT_VIDEO);
    if (!mute && !fw.audio.open()) std::puts("no sound device; running silent");
    SDL_Window* window = SDL_CreateWindow("BayuPandu simulator", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, kWidth * scale,
                                          kHeight * scale, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, kWidth, kHeight);

    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::puts("Keys: Up/Down = previous/next page, Enter = edit (Settings),");
    std::puts("      Esc/Backspace = CANCEL (back to Flight), S = screenshot, Q = quit");
    std::puts("Faults: G = GPS lost on/off, D = data lost on/off, B = battery 100/15/3%");
    fw.printFaults();

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
                if (down && !e.key.repeat) {
                    switch (e.key.keysym.sym) {
                        case SDLK_g: fw.sim().setGpsLost(!fw.sim().isGpsLost()); fw.printFaults(); break;
                        case SDLK_d: fw.sim().setBaroLost(!fw.sim().isBaroLost()); fw.printFaults(); break;
                        case SDLK_b: {
                            int pct = fw.power.getPercent();
                            fw.setBattery(pct > 15 ? 15 : pct > 3 ? 3 : 100);
                            fw.printFaults();
                            break;
                        }
                        default: break;
                    }
                }
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
    std::string bootDir;
    std::string chimeDir;
    std::string tracePath;
    std::string cardDir;
    bool mute = false;
    double atSeconds = 60.0;
    std::string fromText;
    long lengthSec = 180;
    double speed = 1.0;
    int scale = 2;
    uint8_t language = 0;
    bool noGps = false, noData = false, gpsSilent = false;
    int battery = 100;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        auto next = [&]() -> const char* { return i + 1 < argc ? argv[++i] : ""; };
        if (arg == "--shots") shotsDir = next();
        else if (arg == "--boot-frames") bootDir = next();
        else if (arg == "--export-chime") chimeDir = next();
        else if (arg == "--trace") tracePath = next();
        else if (arg == "--card") cardDir = next();
        else if (arg == "--mute") mute = true;
        else if (arg == "--at") atSeconds = std::atof(next());
        else if (arg == "--speed") speed = std::atof(next());
        else if (arg == "--scale") scale = std::atoi(next());
        else if (arg == "--lang") language = std::string(next()) == "id" ? 1 : 0;
        else if (arg == "--from") fromText = next();
        else if (arg == "--length") lengthSec = parseClock(next());
        else if (arg == "--no-gps") noGps = true;
        else if (arg == "--no-data") noData = true;
        else if (arg == "--gps-silent") gpsSilent = true;
        else if (arg == "--battery") battery = std::atoi(next());
        else if (arg == "-h" || arg == "--help") {
            std::puts("usage: bayupandu_sim [file.igc] [--from TIME] [--length TIME] [--speed N] [--scale N] [--lang en|id]\n"
                      "                     [--no-gps] [--gps-silent] [--no-data] [--battery PCT]\n"
                      "       bayupandu_sim [file.igc] [--from TIME] [--length TIME] --shots DIR [--at SECONDS]\n"
                      "\n"
                      "Replays --length (default 3:00, 0 = whole flight) starting --from after the first\n"
                      "fix, looping. TIME is SECONDS, MM:SS or H:MM:SS. With no file, plays a thermal\n"
                      "and glide from the bundled example flight.\n"
                      "\n"
                      "--no-gps (fix lost), --gps-silent (module not answering), --no-data and\n"
                      "--battery start with that fault; in the window,\n"
                      "G, D and B toggle them. --shots also captures each warning state.\n"
                      "--card DIR is the simulated SD card: flight logs go to DIR/flights and the\n"
                      "header's pilot and glider come from DIR/pilot.txt (default: the sim folder;\n"
                      "headless runs use a temporary card unless --card is given).\n"
                      "--trace FILE writes the replay's climb, raw and through the vario filter, as CSV.\n"
                      "--boot-frames DIR writes the boot sequence as 25 fps PNG frames.\n"
                      "--export-chime DIR writes the startup chime as .mid and .wav; --mute silences the window.");
            return 0;
        } else igcPath = arg;
    }

    if (!chimeDir.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(chimeDir, ec);
        return exportChime(chimeDir);
    }

    bool headless = !shotsDir.empty() || !bootDir.empty() || !tracePath.empty();
    if (cardDir.empty()) {
        cardDir = headless ? (std::filesystem::temp_directory_path() / "bayupandu-card").string()
                           : BAYUPANDU_SIM_CARD;
    }
    Firmware fw(cardDir);
    fw.setup();
    fw.configService->getConfig().language = language;

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
    fw.sim().setGpsLost(noGps);
    fw.sim().setBaroLost(noData);
    fw.sim().setGpsSilent(gpsSilent);
    fw.setBattery(battery);
    std::printf("Flight logs go to %s/flights\n", cardDir.c_str());
    std::printf("Replaying %s from +%ld s for %s, looping\n", igcPath.c_str(), fromSec,
                lengthSec > 0 ? (std::to_string(lengthSec) + " s").c_str() : "the whole flight");

    for (const std::string& dir : {shotsDir, bootDir}) {
        std::error_code ec;
        if (!dir.empty()) std::filesystem::create_directories(dir, ec);
    }
    if (!bootDir.empty()) return runBootFrames(fw, bootDir);
    if (!tracePath.empty()) return runTrace(fw, tracePath, lengthSec > 0 ? lengthSec : 180);
    return shotsDir.empty() ? runInteractive(fw, speed, scale, mute)
                            : runHeadless(fw, shotsDir, atSeconds);
}

#pragma once

#include "Data/Types.h"
#include "UI/UiStrings.h"
#include "lvgl.h"

// The startup sign. Wind comes in low from the left, meets a ridge and turns
// upward; the wing mark rides that lift one step higher for every self-test result
// shown below the ridge. A failed check is a yellow line, held a little longer.
// Any button skips it. It owns one LVGL screen, deleted with it.
class BootScreen {
public:
    BootScreen(const UiStrings& text, const SelfTestResult& result, uint32_t nowMs);
    ~BootScreen();
    BootScreen(const BootScreen&) = delete;
    BootScreen& operator=(const BootScreen&) = delete;

    lv_obj_t* screen() const { return root; }

    // Advances the animation. Returns false once the sequence has finished.
    bool update(uint32_t nowMs);
    void skip() { skipped = true; }

    static constexpr int kRows = 4;

private:
    struct Row {
        lv_obj_t* band = nullptr;     // yellow, only on a failed check
        lv_obj_t* glyph = nullptr;    // warning diamond, only on a failed check
        lv_obj_t* key = nullptr;
        lv_obj_t* value = nullptr;
        bool warn = false;
    };

    void buildRow(int index, const char* key, const char* value, bool warn);
    static void onDrawArt(lv_event_t* e);
    void drawArt(lv_draw_ctx_t* ctx) const;

    lv_obj_t* root = nullptr;
    lv_obj_t* art = nullptr;
    Row rows[kRows];
    uint32_t startMs;
    uint32_t elapsedMs = 0;
    bool anyWarning = false;
    lv_coord_t keyX = 0, valueX = 0;
    bool skipped = false;
};

#include "RomSelector.h"
#include <dirent.h>
#include <esp_heap_caps.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Log.h"

// Sprite rendered at this size, then scaled up x2 to fill 320x240.
// Same approach as Display::UpdateTexture: small sprite + scale > 1.
// Dimensions chosen so scale = min(320/160, 240/116) = 2.0 (width-limited),
// ensuring source y=0 maps to dest y = 120 + 116 = 236 < 240 (no clipping).
static constexpr int SELECTOR_W = 160;
static constexpr int SELECTOR_H = 116;

// RGB565 palette
static constexpr uint16_t C_BG      = 0x0000;
static constexpr uint16_t C_TITLEBG = 0x0003;
static constexpr uint16_t C_TITLE   = 0xFFE0;
static constexpr uint16_t C_TEXT    = 0xBDF7;
static constexpr uint16_t C_SEL_BG  = 0x07E0;
static constexpr uint16_t C_SEL_TXT = 0x0000;
static constexpr uint16_t C_HINT    = 0x8410;
static constexpr uint16_t C_LINE    = 0x2104;
static constexpr uint16_t C_SCROLL  = 0x4208;

// ReadKeyboard() is indexed by the key's real hex value (e.g. '2' -> 0x2), not by its
// physical wiring position, so these match the hex digit of the chosen keys directly.
static constexpr int KEY_UP     = 0x2;
static constexpr int KEY_DOWN   = 0x8;
static constexpr int KEY_SELECT = 0x5;

// Layout at SELECTOR_W x SELECTOR_H using textSize(1) — Font0 6x8.
// After x2 scale, chars appear as 12x16 on screen.
static constexpr int TITLE_H = 12;
static constexpr int ITEM_H  = 10;
static constexpr int HINT_H  = 10;
static constexpr int MARGIN  = 3;
static constexpr int CHAR_W  = 6;
static constexpr int CHAR_H  = 8;

static std::string DisplayName(const std::string& fullPath)
{
    size_t slash = fullPath.rfind('/');
    std::string name = (slash != std::string::npos) ? fullPath.substr(slash + 1) : fullPath;
    size_t dot = name.rfind('.');
    if (dot != std::string::npos) name = name.substr(0, dot);
    return name;
}

std::vector<std::string> RomSelector::ListRoms(const std::string& romsDir)
{
    std::vector<std::string> roms;
    DIR* dir = opendir(romsDir.c_str());
    if (!dir) {
        LOGE("{RomSelector} Cannot open: %s", romsDir.c_str());
        return roms;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") continue;
        size_t dot = name.rfind('.');
        if (dot == std::string::npos) continue;
        std::string ext = name.substr(dot);
        for (char& c : ext) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        if (ext == ".ch8" || ext == ".c8" || ext == ".sc8" || ext == ".c8x")
            roms.push_back(romsDir + "/" + name);
    }
    closedir(dir);
    std::sort(roms.begin(), roms.end());
    return roms;
}

// Mirrors Display::UpdateTexture exactly:
//   scale = min(panelW / spriteW, panelH / spriteH)
//   scaleY = isFlipY ? -scale : scale
//   pushImageRotateZoom(panelW/2, panelH/2, spriteW/2, spriteH/2, 0, scaleX, scaleY, spriteW, spriteH, buf)
void RomSelector::PushToPanel(lgfx::LGFX_Sprite& sprite, FisicalPanel& panel, bool isFlipY, bool isFlipX)
{
    const float W  = static_cast<float>(sprite.width());
    const float H  = static_cast<float>(sprite.height());
    const float pW = static_cast<float>(panel.width());
    const float pH = static_cast<float>(panel.height());

    isFlipX = 1;
    isFlipY = 0;

    const float scaleX = isFlipX ? -(pW / W) : (pW / W);
    const float scaleY = isFlipY ? -(pH / H) : (pH / H);

    panel.pushImageRotateZoom(
        pW / 2.0f, pH / 2.0f,
        W  / 2.0f, H  / 2.0f,
        0.0f,
        scaleX, scaleY,
        static_cast<int>(W), static_cast<int>(H),
        static_cast<uint16_t*>(sprite.getBuffer())
    );
}

void RomSelector::Render(lgfx::LGFXBase& cv, const std::vector<std::string>& roms,
                         int selected, int scrollOffset, int maxVisible)
{
    const int W     = cv.width();
    const int H     = cv.height();
    const int total = static_cast<int>(roms.size());

    cv.fillScreen(C_BG);

    // Title bar
    cv.fillRect(0, 0, W, TITLE_H, C_TITLEBG);
    cv.setTextSize(1);
    cv.setTextColor(C_TITLE, C_TITLEBG);
    cv.drawString("CHIP-8 ROM SELECTOR", MARGIN, (TITLE_H - CHAR_H) / 2);

    char countBuf[32];
    snprintf(countBuf, sizeof(countBuf), "%d/%d", selected + 1, total);
    int countX = W - static_cast<int>(strlen(countBuf)) * CHAR_W - MARGIN;
    cv.drawString(countBuf, countX, (TITLE_H - CHAR_H) / 2);

    cv.drawFastHLine(0, TITLE_H, W, C_LINE);

    // ROM list
    const int listY   = TITLE_H + 1;
    const int listW   = W - 8;
    const int maxChar = (listW - MARGIN) / CHAR_W;

    cv.setTextSize(1);
    for (int i = 0; i < maxVisible; i++) {
        int idx = scrollOffset + i;
        if (idx >= total) break;
        int  y     = listY + i * ITEM_H;
        bool isSel = (idx == selected);

        if (isSel) {
            cv.fillRect(0, y, listW, ITEM_H, C_SEL_BG);
            cv.setTextColor(C_SEL_TXT, C_SEL_BG);
        } else {
            cv.setTextColor(C_TEXT, C_BG);
        }

        std::string name = DisplayName(roms[idx]);
        if (static_cast<int>(name.size()) > maxChar)
            name = name.substr(0, maxChar - 3) + "...";
        cv.drawString(name.c_str(), MARGIN, y + (ITEM_H - CHAR_H) / 2);
    }

    // Scrollbar
    if (total > maxVisible) {
        const int sbX   = W - 6;
        const int listH = maxVisible * ITEM_H;
        cv.fillRect(sbX, listY, 4, listH, C_SCROLL);
        int thumbH = std::max(4, listH * maxVisible / total);
        int thumbY = listY + (listH - thumbH) * scrollOffset / (total - maxVisible);
        cv.fillRect(sbX, thumbY, 4, thumbH, C_HINT);
    }

    // Hint bar
    const int hintY = H - HINT_H;
    cv.drawFastHLine(0, hintY - 1, W, C_LINE);
    cv.setTextSize(1);
    cv.setTextColor(C_HINT, C_BG);
    cv.drawString("2=UP  8=DOWN  5=SELECT", MARGIN, hintY + (HINT_H - CHAR_H) / 2);
}

std::string RomSelector::Select(const std::string& romsDir, FisicalPanel& panel,
                                Keyboard* keyboard, bool isFlipY, bool isFlipX)
{
    const int maxVisible = (SELECTOR_H - TITLE_H - HINT_H - 3) / ITEM_H;

    LOG("{RomSelector} Free heap: %lu bytes, largest block: %lu bytes",
        (unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT),
        (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    lgfx::LGFX_Sprite sprite(&panel);
    sprite.setColorDepth(16);
    sprite.createSprite(SELECTOR_W, SELECTOR_H);
    const bool useSprite = (sprite.getBuffer() != nullptr);

    LOG("{RomSelector} Sprite %s (%dx%d, needs %d bytes)",
        useSprite ? "OK" : "FAILED",
        SELECTOR_W, SELECTOR_H,
        SELECTOR_W * SELECTOR_H * 2);

    lgfx::LGFXBase& cv = useSprite ? static_cast<lgfx::LGFXBase&>(sprite)
                                   : static_cast<lgfx::LGFXBase&>(panel);

    auto flush = [&]() { if (useSprite) PushToPanel(sprite, panel, isFlipY, isFlipX); };

    auto roms = ListRoms(romsDir);

    if (roms.empty()) {
        cv.fillScreen(C_BG);
        cv.setTextSize(1);
        cv.setTextColor(0xF800, C_BG);
        cv.drawString("No ROMs found:", MARGIN, cv.height() / 2 - CHAR_H);
        cv.setTextColor(C_HINT, C_BG);
        cv.drawString(romsDir.c_str(), MARGIN, cv.height() / 2 + 2);
        flush();
        vTaskDelay(pdMS_TO_TICKS(5000));
        return "";
    }

    int selected     = 0;
    int scrollOffset = 0;

    Render(cv, roms, selected, scrollOffset, maxVisible);
    flush();

    if (!keyboard) {
        cv.setTextSize(1);
        for (int secs = 5; secs > 0; secs--) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Auto-loading in %d...  ", secs);
            cv.fillRect(0, cv.height() - HINT_H, cv.width(), HINT_H, C_BG);
            cv.setTextColor(C_TITLE, C_BG);
            cv.drawString(buf, MARGIN, cv.height() - HINT_H + (HINT_H - CHAR_H) / 2);
            flush();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        LOG("{RomSelector} Auto-loading: %s", roms[0].c_str());
        return roms[0];
    }

    bool prevUp = false, prevDown = false, prevSel = false;

    while (true) {
        keyboard->ProcessInput();

        bool up   = keyboard->ReadKeyboard(KEY_UP)     != 0;
        bool down = keyboard->ReadKeyboard(KEY_DOWN)   != 0;
        bool sel  = keyboard->ReadKeyboard(KEY_SELECT) != 0;

        bool changed = false;

        if (up && !prevUp) {
            if (selected > 0) {
                selected--;
                if (selected < scrollOffset) scrollOffset = selected;
                changed = true;
            }
        } else if (down && !prevDown) {
            if (selected < static_cast<int>(roms.size()) - 1) {
                selected++;
                if (selected >= scrollOffset + maxVisible)
                    scrollOffset = selected - maxVisible + 1;
                changed = true;
            }
        }

        if (sel && !prevSel) {
            LOG("{RomSelector} Selected: %s", roms[selected].c_str());
            return roms[selected];
        }

        prevUp   = up;
        prevDown = down;
        prevSel  = sel;

        if (changed) {
            Render(cv, roms, selected, scrollOffset, maxVisible);
            flush();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

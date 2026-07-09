#include "RomSelector.h"
#include <dirent.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Log.h"

// 96x72 * 2 = 13824 bytes. Scale factor: 320/96 = 240/72 = 3.333 (4:3, no letterboxing).
static constexpr int SELECTOR_W = 96;
static constexpr int SELECTOR_H = 72;

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
static constexpr int KEY_UP     = 0x5;
static constexpr int KEY_DOWN   = 0x8;
static constexpr int KEY_SELECT = 0xE;

// Layout at SELECTOR_W x SELECTOR_H using textSize(1) -- Font0 6x8.
// After 3.33x scale, chars appear as ~20x27 on screen (readable).
static constexpr int TITLE_H = 10;
static constexpr int ITEM_H  = 9;
static constexpr int HINT_H  = 9;
static constexpr int MARGIN  = 2;
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

// Mirrors Display::UpdateTexture: negative scaleY when isFlipY, negative scaleX when isFlipX.
// pW/pH must be the physical panel size (320x240) — never panel.width()/height(), which
// may return swapped values depending on the current rotation setting.
void RomSelector::PushToPanel(lgfx::LGFX_Sprite& sprite, FisicalPanel& panel, bool isFlipY, bool isFlipX,
                               float pW, float pH)
{
    const float sW     = static_cast<float>(sprite.width());
    const float sH     = static_cast<float>(sprite.height());
    const float scaleX = (isFlipX ? -1.0f : 1.0f) * (pW / sW);
    const float scaleY = (isFlipY ? -1.0f : 1.0f) * (pH / sH);

    sprite.setPivot(sW / 2.0f, sH / 2.0f);
    sprite.pushRotateZoom(&panel, pW / 2.0f, pH / 2.0f, 0.0f, scaleX, scaleY);
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
    cv.drawString("CHIP-8", MARGIN, (TITLE_H - CHAR_H) / 2);

    char countBuf[24];
    snprintf(countBuf, sizeof(countBuf), "%d/%d", selected + 1, total);
    int countX = W - static_cast<int>(strlen(countBuf)) * CHAR_W - MARGIN;
    cv.drawString(countBuf, countX, (TITLE_H - CHAR_H) / 2);

    cv.drawFastHLine(0, TITLE_H, W, C_LINE);

    // ROM list
    const int listY   = TITLE_H + 1;
    const int listW   = W - 6;
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
        const int sbX   = W - 4;
        const int listH = maxVisible * ITEM_H;
        cv.fillRect(sbX, listY, 3, listH, C_SCROLL);
        int thumbH = std::max(3, listH * maxVisible / total);
        int thumbY = listY + (listH - thumbH) * scrollOffset / (total - maxVisible);
        cv.fillRect(sbX, thumbY, 3, thumbH, C_HINT);
    }

    // Hint bar
    const int hintY = H - HINT_H;
    cv.drawFastHLine(0, hintY - 1, W, C_LINE);
    cv.setTextSize(1);
    cv.setTextColor(C_HINT, C_BG);
    cv.drawString("2=UP 8=DN 5=OK", MARGIN, hintY + (HINT_H - CHAR_H) / 2);
}

std::string RomSelector::Select(const std::string& romsDir, FisicalPanel& panel,
                                Keyboard* keyboard, bool isFlipY, bool isFlipX)
{
    const int maxVisible = (SELECTOR_H - TITLE_H - HINT_H - 2) / ITEM_H;

    lgfx::LGFX_Sprite sprite(&panel);
    sprite.setColorDepth(16);
    sprite.createSprite(SELECTOR_W, SELECTOR_H);
    const bool useSprite = (sprite.getBuffer() != nullptr);

    lgfx::LGFXBase& cv = useSprite ? static_cast<lgfx::LGFXBase&>(sprite)
                                   : static_cast<lgfx::LGFXBase&>(panel);

    // flush pushes the sprite to the panel using the same transform as the game:
    // positive scaleX, negative scaleY when isFlipY=true (config flipY=1).
    auto flush = [&]() {
        if (useSprite) PushToPanel(sprite, panel, isFlipY, isFlipX);
    };

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
            snprintf(buf, sizeof(buf), "Auto: %d...  ", secs);
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

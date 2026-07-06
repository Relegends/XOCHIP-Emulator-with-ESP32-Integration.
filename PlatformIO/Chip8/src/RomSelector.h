#pragma once
#include <string>
#include <vector>
#include "FisicalPanel.h"
#include "Keyboard.h"

class RomSelector
{
public:
    std::string Select(const std::string& romsDir, FisicalPanel& panel, Keyboard* keyboard, bool isFlipY = false, bool isFlipX = false);

private:
    std::vector<std::string> ListRoms(const std::string& romsDir);
    void Render(lgfx::LGFXBase& canvas, const std::vector<std::string>& roms,
                int selected, int scrollOffset, int maxVisible);
    void PushToPanel(lgfx::LGFX_Sprite& sprite, FisicalPanel& panel, bool isFlipY, bool isFlipX);
};

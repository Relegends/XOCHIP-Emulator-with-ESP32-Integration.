#include "FisicalPanel.h"

FisicalPanel::FisicalPanel(const unsigned int panelWidth, const unsigned int panelHeight)
{
    auto busConfig = m_bus.config();

    busConfig.freq_write = 40000000;    // Works better in a mounted board.
    busConfig.freq_read = 16000000;     // Tactile function.
    busConfig.pin_sclk = 18;            // GPIO 18. SPI clock.
    busConfig.pin_miso = 19;            // GPIO 19. MISO (Master In Slave Out) use for tactile function.
    busConfig.pin_mosi = 23;            // GPIO 23. MOSI (Manster Out Slave In) use to write data from CPU to screen.
    busConfig.pin_dc = 2;
    busConfig.spi_mode = 0;             // SPI mode 0. Better for TFT screens.
    busConfig.spi_host = VSPI_HOST;     // VSPI_HOST use SPI 2. More common and better when using flash memory.

    m_bus.config(busConfig);
    m_panel.setBus(&m_bus);

    auto panelConfig = m_panel.config();
    panelConfig.panel_width = panelWidth;               // 320 pixel
    panelConfig.panel_height = panelHeight;             // 240 pixel
    panelConfig.pin_cs = 5;                             // GPIO 5. CS (Chip Select). 
    panelConfig.pin_rst = 4;                            // GPIO 4. RST (Display reset).
    panelConfig.offset_x = panelConfig.offset_y = 0;    // NOT OFFSET. Full Screen mode.
    panelConfig.invert = true;                          // ST7790: Set to true to ensure m_combineBuffer in Octo mode is shown correctly.
    panelConfig.rgb_order = false;                      // ORDER: RED, GREEN, BLUE.

    m_panel.config(panelConfig);
    setPanel(&m_panel);
}
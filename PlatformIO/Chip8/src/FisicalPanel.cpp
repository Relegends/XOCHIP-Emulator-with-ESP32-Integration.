#include "FisicalPanel.h"

FisicalPanel::FisicalPanel()
{
    m_panelWidth = 0;
    m_panelHeight = 0;
}

bool FisicalPanel::Init(const unsigned int panelWidth, const unsigned int panelHeight, const unsigned int panelRotation, const bool rgbOrder, const bool flipY)
{
    LOG("{FisicalPanel} Initializing panel with width: %d and height: %d", panelWidth, panelHeight);
    LOG("{FisicalPanel} Initializing panel with orientation: %d, rgb order: %d, flipY: %d", panelRotation, rgbOrder, flipY);

    m_panelWidth = panelWidth;
    m_panelHeight = panelHeight;
    m_panelRotation = panelRotation;
    m_rgbOrder = rgbOrder;
    m_flipY = flipY;

    InitBus();
    InitPanel();
    InitBackLight();
    
    if (!init()) // Init the panel from LovyanGFX, if it returns false, there was an error initializing the panel
	{
		LOG("{FisicalPanel} ERROR! Panel init went wrong!");
        return false;
	}

    // URJC 0
    setRotation(m_panelRotation);
    setSwapBytes(true);

    LOG("{FisicalPanel} Initialized successfully!");

    return true;
}

void FisicalPanel::InitBus()
{
    auto busConfig = m_bus.config();

    busConfig.freq_write = 80000000;    // Works better in a mounted board.
    busConfig.freq_read = 16000000;     // Tactile function.
    busConfig.pin_sclk = GPIO_NUM_14;   // GPIO 18. SPI clock.
    busConfig.pin_miso = GPIO_NUM_12;   // GPIO 19. MISO (Master In Slave Out) use for tactile function.
    busConfig.pin_mosi = GPIO_NUM_13;   // GPIO 23. MOSI (Manster Out Slave In) use to write data from CPU to screen.
    busConfig.pin_dc = GPIO_NUM_2;
    busConfig.spi_mode = 0;             // SPI mode 0. Better for TFT screens.
    busConfig.spi_host = HSPI_HOST;     // Using HSPI (SPI2) to avoid conflict with SD card on SPI3.

    m_bus.config(busConfig);
    m_panel.setBus(&m_bus);
}

void FisicalPanel::InitBackLight()
{
    auto lightConfig = m_light.config();
    lightConfig.pin_bl =  GPIO_NUM_21;
    lightConfig.invert = false;
    lightConfig.pwm_channel = 1;

    m_light.config(lightConfig);
    
    m_light.init(255);  // TODO: Add to config

    m_panel.setLight(&m_light);
}

void FisicalPanel::InitPanel()
{
    auto panelConfig = m_panel.config();
    panelConfig.panel_width = m_panelWidth;               // 320 pixel
    panelConfig.panel_height = m_panelHeight;             // 240 pixel
    panelConfig.pin_cs = GPIO_NUM_15;                             // GPIO 5. CS (Chip Select). 
    panelConfig.pin_rst = GPIO_NUM_4;                            // GPIO 4. RST (Display reset).
    panelConfig.offset_x = panelConfig.offset_y = 0;    // NOT OFFSET. Full Screen mode.
    panelConfig.invert = false;                          // HIGH = ON
    panelConfig.rgb_order = m_rgbOrder;                      // ORDER: RED, GREEN, BLUE.
    panelConfig.readable = true;

    m_panel.config(panelConfig);

    m_panel.setTouch(nullptr);

    setPanel(&m_panel);
}
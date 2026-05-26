#include "Storage.h"
#include "Log.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

Storage::Storage(const std::string& mountPoint)
    : m_mountPoint(mountPoint),
      m_isMounted(false),
      m_card(nullptr)
{
}

Storage::~Storage()
{
    Unmount();
}

bool Storage::Mount()
{
    LOG("{Storage} Mounting SD at %s", m_mountPoint.c_str());

    // SD pins CYD
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = GPIO_NUM_23;
    bus_cfg.miso_io_num = GPIO_NUM_19;
    bus_cfg.sclk_io_num = GPIO_NUM_18;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;

    esp_err_t err = spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        LOGE("{Storage} SPI init failed: %s", esp_err_to_name(err));
        return false;
    }

    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_5, 1);

    vTaskDelay(pdMS_TO_TICKS(200));

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    host.max_freq_khz = 1000;
    host.flags = SDMMC_HOST_FLAG_SPI;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_NUM_5;
    slot_config.host_id = SPI3_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;
    mount_config.allocation_unit_size = 16 * 1024;

    err = esp_vfs_fat_sdspi_mount(
        m_mountPoint.c_str(),
        &host,
        &slot_config,
        &mount_config,
        &m_card
    );

    if (err != ESP_OK) {
        LOGE("{Storage} SD mount failed: %s", esp_err_to_name(err));
        return false;
    }

    m_isMounted = true;
    LOG("{Storage} SD mounted OK");
    return true;
}

void Storage::Unmount()
{
    if (!m_isMounted) return;

    esp_err_t err = esp_vfs_fat_sdcard_unmount(m_mountPoint.c_str(), m_card);

    if (err != ESP_OK) {
        LOGE("{Storage} Unmount failed: %s", esp_err_to_name(err));
    }

    m_card = nullptr;
    m_isMounted = false;

    LOG("{Storage} SD unmounted");
}

bool Storage::IsMounted() const
{
    return m_isMounted;
}
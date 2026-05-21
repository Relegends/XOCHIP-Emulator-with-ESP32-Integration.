#include "Storage.h"

Storage::Storage(const std::string& mountPoint)
    : m_mountPoint(mountPoint), m_isMounted(false)
{
}

Storage::~Storage()
{
    if (m_isMounted) {
        Unmount();
    }
}

bool Storage::Mount()
{
    LOG("Mounting storage at %s...", m_mountPoint.c_str());

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = 12; // GPIO 12 for MISO
    slot_config.gpio_mosi = 13; // GPIO 13 for MOSI
    slot_config.gpio_sck = 14;  // GPIO 14 for SCK
    slot_config.gpio_cs = 15;   // GPIO 15 for CS
    slot_config.host_id = HSPI_HOST;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t* card = nullptr;

    esp_err_t err = esp_vfs_fat_sdspi_mount(m_mountPoint.c_str(), &host, &slot_config, &mount_config, &card);

    if (err != ESP_OK) {
        LOGE("Failed to mount storage (0x%x)", err);
        return false;
    }

    m_isMounted = true;
    LOG("Storage mounted successfully at %s", m_mountPoint.c_str());

    return true;
}

void Storage::Unmount()
{
    if (m_isMounted) {
        esp_vfs_fat_sdmmc_unmount();
        m_isMounted = false;
        LOG("Storage unmounted from %s", m_mountPoint.c_str());
    }
}

bool Storage::IsMounted() const
{
    return m_isMounted;
}




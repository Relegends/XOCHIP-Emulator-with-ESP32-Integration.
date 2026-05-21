#pragma once
#include <string>
#include <sdmmc_cmd.h>
#include <esp_vfs_fat.h>
#include <driver/sdspi_host.h>
#include "Log.h"


class Storage
{
public:
    Storage(const std::string& mountPoint);
    ~Storage();

    bool Mount();
    void Unmount();
    bool IsMounted() const;

private:
    std::string m_mountPoint;
    bool m_isMounted;
};
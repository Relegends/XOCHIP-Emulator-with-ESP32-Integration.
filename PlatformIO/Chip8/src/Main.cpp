#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Log.h>
#include "Chip8.h"
#include "Storage.h"
#include <dirent.h>
#include <driver/i2c_master.h>

void i2c_scan()
{
    ESP_LOGI("I2C", "Scanning I2C bus...");

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_27,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    i2c_master_bus_handle_t bus_handle = nullptr;

    esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus_handle);

    if (err != ESP_OK) {
        ESP_LOGE("I2C", "Failed to create I2C bus: %s", esp_err_to_name(err));
        return;
    }

    bool found = false;

    for (uint8_t addr = 1; addr < 127; addr++) {

        err = i2c_master_probe(bus_handle, addr, pdMS_TO_TICKS(100));

        if (err == ESP_OK) {
            ESP_LOGI("I2C", "Device found at 0x%02X", addr);
            found = true;
        }
    }

    if (!found) {
        ESP_LOGW("I2C", "No I2C devices found");
    }

    i2c_del_master_bus(bus_handle);

    ESP_LOGI("I2C", "Scan completed");
}

void list_dir(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        LOGE("{Main} Cannot open directory %s", path);
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        LOG("{Main}  %s/%s", path, entry->d_name);
    }
    closedir(dir);
}

extern "C" void app_main() 
{
    LOG("{Main} Starting CHIP 8 Emulator...");

	i2c_scan();

	Storage storage("/sdcard");
	if (!storage.Mount()) {
		LOGE("{Main} Failed to mount storage. Exiting...");
		abort();
	}

	vTaskDelay(pdMS_TO_TICKS(1000));
	LOG("{Main} Listing SD contents:");
	list_dir("/sdcard");
	list_dir("/sdcard/roms");

	auto emulator = std::make_unique<Chip8>();

	bool emulatorInit = emulator->Init("/sdcard/config/Config.toml");

	std::string romFile = "/sdcard/roms/1-chip8-logo.ch8";
	LOG("{Main} Loading ROM: %s", romFile.c_str());

	bool gameLoaded = emulator->LoadGame(romFile);

	if (emulatorInit && gameLoaded)
		emulator->Run();
	else
		LOGE("{Main} Failed to run emulator. Exiting...");
	
	LOG("{Main} Bye Bye!");

	while (true) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
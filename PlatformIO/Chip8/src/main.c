#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Log.h>
#include "Chip8.h"
#include "Storage.h"

extern "C" void app_main() 
{
    LOG("Starting CHIP 8 Emulator...");

	Storage storage("/sdcard");
	if (!storage.Mount()) {
		LOGE("Failed to mount storage. Exiting...");
		return;
	}

	const char* romFile = "/sdcard/roms/pong.ch8";
	LOG("Loading ROM: %s", romFile);

	Chip8 emulator;

	emulator.LoadGame(romFile);

	emulator.Run();

	storage.Unmount();
	
	LOG("Bye Bye!");
}
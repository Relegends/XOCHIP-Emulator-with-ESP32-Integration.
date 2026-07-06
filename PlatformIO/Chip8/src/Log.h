#pragma once
#include <esp_log.h>

#define LOG(...)	ESP_LOGI("INFO", ##__VA_ARGS__)
#define LOGE(...)	ESP_LOGE("ERROR", ##__VA_ARGS__)
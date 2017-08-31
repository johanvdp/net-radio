// The author disclaims copyright to this source code.
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "blink.h"
#include "test_mem.h"
#include "test_dsp.h"
#include "test_buffer.h"
#include "reader.h"
#include "player.h"
#include "factory.h"
#include "statistics.h"

static const char* TAG = "main.c";

spi_mem_handle_t main_spi_mem_handle;
buffer_handle_t main_buffer_handle;
vs1053_handle_t main_vs1053_handle;
reader_config_t main_reader_configuration;
player_config_t main_player_configuration;
test_mem_config_t main_test_mem_configuration;
test_dsp_config_t main_test_dsp_configuration;
test_buffer_config_t main_test_buffer_configuration;
statistics_config_t main_statistics_configuration;

void main_log_configuration() {
	ESP_LOGD(TAG, ">main_log_configuration");
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	if (chip_info.model == CHIP_ESP32) {
		ESP_LOGI(TAG, "model: ESP32");
	} else {
		ESP_LOGI(TAG, "model: %d (unknown)", chip_info.model);
	}
	ESP_LOGI(TAG, "cores: %d", chip_info.cores);
	ESP_LOGI(TAG, "features WiFi: %s", ((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features BT: %s", ((chip_info.features & CHIP_FEATURE_BT) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features BLE: %s", ((chip_info.features & CHIP_FEATURE_BLE) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features flash: %s", ((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"));
	ESP_LOGI(TAG, "flash size (MB): %d", (spi_flash_get_chip_size() / (1024 * 1024)));
	ESP_LOGI(TAG, "revision: %d", chip_info.revision);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_CLK: %d", CONFIG_GPIO_VSPI_CLK);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_MOSI: %d", CONFIG_GPIO_VSPI_MOSI);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_MISO: %d", CONFIG_GPIO_VSPI_MISO);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_CLK: %d", CONFIG_GPIO_HSPI_CLK);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_MOSI: %d", CONFIG_GPIO_HSPI_MOSI);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_MISO: %d", CONFIG_GPIO_HSPI_MISO);
	ESP_LOGD(TAG, "<main_log_configuration");
}

void main_vspi_initialize() {
	ESP_LOGD(TAG, ">main_vspi_initialize");
	spi_bus_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.sclk_io_num = CONFIG_GPIO_VSPI_CLK;
	configuration.mosi_io_num = CONFIG_GPIO_VSPI_MOSI;
	configuration.miso_io_num = CONFIG_GPIO_VSPI_MISO;
	configuration.quadwp_io_num = -1;
	configuration.quadhd_io_num = -1;
	ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &configuration, 1));
	ESP_LOGD(TAG, "<main_vspi_initialize");
}

void main_vspi_free() {
	ESP_LOGD(TAG, ">main_vspi_free");
	ESP_ERROR_CHECK(spi_bus_free(VSPI_HOST));
	ESP_LOGD(TAG, "<main_vspi_free");
}

void main_hspi_initialize() {
	ESP_LOGD(TAG, ">main_hspi_initialize");
	spi_bus_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.sclk_io_num = CONFIG_GPIO_HSPI_CLK;
	configuration.mosi_io_num = CONFIG_GPIO_HSPI_MOSI;
	configuration.miso_io_num = CONFIG_GPIO_HSPI_MISO;
	configuration.quadwp_io_num = -1;
	configuration.quadhd_io_num = -1;
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &configuration, 2));
	ESP_LOGD(TAG, "<main_hspi_initialize");
}

void main_hspi_free() {
	ESP_LOGD(TAG, ">main_hspi_free");
	ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
	ESP_LOGD(TAG, "<main_hspi_free");
}

void main_handles_create() {
	ESP_LOGD(TAG, ">main_handles_create");
	factory_mem_create(&main_spi_mem_handle);
	factory_buffer_create(main_spi_mem_handle, CONFIG_MEM_TOTAL_BYTES, &main_buffer_handle);
	factory_dsp_create(&main_vs1053_handle);
	ESP_LOGD(TAG, "main_spi_mem_handle: %p", main_spi_mem_handle);
	ESP_LOGD(TAG, "main_buffer_handle: %p", main_buffer_handle);
	ESP_LOGD(TAG, "main_vs1053_handle: %p", main_vs1053_handle);
	ESP_LOGD(TAG, "<main_handles_create");
}

/**
 * FreeRTOS Application entry point.
 */
void app_main() {
	ESP_LOGD(TAG, ">app_main");

	main_log_configuration();

	main_vspi_initialize();
	main_hspi_initialize();
	main_handles_create();

	main_test_buffer_configuration.buffer_handle = main_buffer_handle;
	if (test_buffer(main_test_buffer_configuration) != ESP_OK) {
		return;
	}

	//main_test_mem_configuration.spi_mem_handle = main_spi_mem_handle;
	//xTaskCreatePinnedToCore(&test_mem_task, "test_mem_task", 4096, &main_test_mem_configuration, 5, NULL, 0);

	main_test_dsp_configuration.vs1053_handle = main_vs1053_handle;
	if (test_dsp(main_test_dsp_configuration) != ESP_OK) {
		return;
	}

	// blink task
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);

	// reader task
	main_reader_configuration.buffer_handle = main_buffer_handle;
	xTaskCreatePinnedToCore(&reader_task, "reader_task", 4096, &main_reader_configuration, 5, NULL, 1);

	// player task
	main_player_configuration.buffer_handle = main_buffer_handle;
	main_player_configuration.vs1053_handle = main_vs1053_handle;
	xTaskCreatePinnedToCore(&player_task, "player_task", 4096, &main_player_configuration, 5, NULL, 0);

	// statistics task
	main_statistics_configuration.buffer_handle = main_buffer_handle;
	xTaskCreate(&statistics_task, "statistics_task", 4096, &main_statistics_configuration, 0, NULL);

	// tasks are still running, never free resources
	//main_vspi_free();
	//main_hspi_free();
	ESP_LOGD(TAG, "<app_main");
}

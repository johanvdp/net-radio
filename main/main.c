// The author disclaims copyright to this source code.
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "factory.h"
#include "test_mem.h"
#include "test_buffer.h"
#include "test_dsp.h"
#include "blink.h"
#include "reader.h"
#include "player.h"
#include "statistics.h"
#include "network.h"
#include "web_server.h"

static const char* TAG = "main";

static spi_mem_handle_t main_spi_mem_handle;
static buffer_handle_t main_buffer_handle;
static vs1053_handle_t main_vs1053_handle;
static reader_config_t main_reader_configuration;
static player_config_t main_player_configuration;
static test_mem_config_t main_test_mem_configuration;
static test_dsp_config_t main_test_dsp_configuration;
static test_buffer_config_t main_test_buffer_configuration;
static statistics_config_t main_statistics_configuration;

static void main_log_configuration() {
	ESP_LOGD(TAG, ">main_log_configuration");
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	if (chip_info.model == CHIP_ESP32) {
		ESP_LOGD(TAG, "model: ESP32");
	} else {
		ESP_LOGD(TAG, "model: %d (unknown)", chip_info.model);
	}
	ESP_LOGD(TAG, "cores: %d", chip_info.cores);
	ESP_LOGD(TAG, "features WiFi: %s", ((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "YES" : "NO"));
	ESP_LOGD(TAG, "features BT: %s", ((chip_info.features & CHIP_FEATURE_BT) ? "YES" : "NO"));
	ESP_LOGD(TAG, "features BLE: %s", ((chip_info.features & CHIP_FEATURE_BLE) ? "YES" : "NO"));
	ESP_LOGD(TAG, "features flash: %s", ((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"));
	ESP_LOGD(TAG, "flash size (MB): %d", (spi_flash_get_chip_size() / (1024 * 1024)));
	ESP_LOGD(TAG, "revision: %d", chip_info.revision);
	ESP_LOGD(TAG, "CONFIG_GPIO_VSPI_CLK: %d", CONFIG_GPIO_VSPI_CLK);
	ESP_LOGD(TAG, "CONFIG_GPIO_VSPI_MOSI: %d", CONFIG_GPIO_VSPI_MOSI);
	ESP_LOGD(TAG, "CONFIG_GPIO_VSPI_MISO: %d", CONFIG_GPIO_VSPI_MISO);
	ESP_LOGD(TAG, "CONFIG_GPIO_HSPI_CLK: %d", CONFIG_GPIO_HSPI_CLK);
	ESP_LOGD(TAG, "CONFIG_GPIO_HSPI_MOSI: %d", CONFIG_GPIO_HSPI_MOSI);
	ESP_LOGD(TAG, "CONFIG_GPIO_HSPI_MISO: %d", CONFIG_GPIO_HSPI_MISO);
	ESP_LOGD(TAG, "<main_log_configuration");
}

static void main_vspi_initialize() {
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

static void main_hspi_initialize() {
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

static void main_handles_create() {
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

	// test memory
	main_test_mem_configuration.spi_mem_handle = main_spi_mem_handle;
	if (test_mem(main_test_mem_configuration) != ESP_OK) {
		return;
	}

	// test buffer (uses memory)
	main_test_buffer_configuration.buffer_handle = main_buffer_handle;
	if (test_buffer(main_test_buffer_configuration) != ESP_OK) {
		return;
	}

	// test dsp
	main_test_dsp_configuration.vs1053_handle = main_vs1053_handle;
	if (test_dsp(main_test_dsp_configuration) != ESP_OK) {
		return;
	}

	ap_begin();
	mdns_begin();

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

	// webserver task
	xTaskCreatePinnedToCore(&web_server_task, "web_server_task", 4096, NULL, 1, NULL, 1);

	// tasks are still running, never free resources
	ESP_LOGD(TAG, "<app_main");
}

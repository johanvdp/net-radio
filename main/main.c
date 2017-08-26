// The author disclaims copyright to this source code.
#include "main.h"

static const char* TAG = "main.c";

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
	ESP_LOGI(TAG, "features WiFi: %s",
			((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features BT: %s",
			((chip_info.features & CHIP_FEATURE_BT) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features BLE: %s",
			((chip_info.features & CHIP_FEATURE_BLE) ? "YES" : "NO"));
	ESP_LOGI(TAG, "features flash: %s",
			((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"));
	ESP_LOGI(TAG, "flash size (MB): %d",
			(spi_flash_get_chip_size() / (1024 * 1024)));
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
/**
 * FreeRTOS Application entry point.
 */
void app_main() {
	main_log_configuration();
	blink_log_configuration();
	mem_log_configuration();
	test_mem_log_configuration();

	main_vspi_initialize();
	main_hspi_initialize();

	xTaskCreate(&test_mem_task, "test_mem_task", 4096, NULL, 5, NULL);
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);

	// tasks are still running, never free resources
	// main_vspi_free();
	// main_hspi_free();
}

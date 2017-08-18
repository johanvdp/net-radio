// The author disclaims copyright to this source code.
#include "main.h"

static const char* TAG = "main.c";

spi_device_handle_t vspi_bus_handle;

void main_log_configuration() {
	ESP_LOGI(TAG, ">main_log_configuration");
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	if (chip_info.model == CHIP_ESP32) {
		ESP_LOGI(TAG, "model: ESP32");
	} else {
		ESP_LOGI(TAG, "model: %d (unknown)", chip_info.model);
	}
	ESP_LOGI(TAG, "cores: %d", chip_info.cores);
	ESP_LOGI(TAG, "features Wifi: %s",
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
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_MISO: %d", CONFIG_GPIO_HSPI_MISO);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_MOSI: %d", CONFIG_GPIO_HSPI_MOSI);
	ESP_LOGI(TAG, "CONFIG_GPIO_HSPI_CLK: %d", CONFIG_GPIO_HSPI_CLK);
	ESP_LOGI(TAG, "CONFIG_GPIO_DSP_XDCS: %d", CONFIG_GPIO_DSP_XDCS);
	ESP_LOGI(TAG, "CONFIG_GPIO_DSP_XCS: %d", CONFIG_GPIO_DSP_XCS);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_MISO: %d", CONFIG_GPIO_VSPI_MISO);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_MOSI: %d", CONFIG_GPIO_VSPI_MOSI);
	ESP_LOGI(TAG, "CONFIG_GPIO_VSPI_CLK: %d", CONFIG_GPIO_VSPI_CLK);
	ESP_LOGI(TAG, "CONFIG_GPIO_I2C_SDA: %d", CONFIG_GPIO_I2C_SDA);
	ESP_LOGI(TAG, "CONFIG_GPIO_I2C_SCL: %d", CONFIG_GPIO_I2C_SCL);
	ESP_LOGI(TAG, "<main_log_configuration");
}

void main_vspi_initialize() {
	ESP_LOGI(TAG, ">main_vspi_initialize");
	spi_bus_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.sclk_io_num = CONFIG_GPIO_VSPI_CLK;
	configuration.mosi_io_num = CONFIG_GPIO_VSPI_MOSI;
	configuration.miso_io_num = CONFIG_GPIO_VSPI_MISO;
	configuration.quadwp_io_num = -1;
	configuration.quadhd_io_num = -1;
	ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &configuration, 0));
	ESP_LOGI(TAG, "<main_vspi_initialize");
}

void main_vspi_free() {
	ESP_LOGI(TAG, ">main_vspi_free");
	ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
	ESP_LOGI(TAG, "<main_vspi_free");
}

void main_task(void *ignore) {

	ESP_LOGI(TAG, "main_task");
	main_vspi_initialize();
	mem_initialize((spi_host_device_t) VSPI_HOST);

	while (1) {
		mem_write_mode_register(MEM_MODE_BYTE);
		uint8_t mode = mem_read_mode_register();
		ESP_LOGI(TAG, "mode: %02x", mode);

		mem_write_mode_register(MEM_MODE_PAGE);
		mode = mem_read_mode_register();
		ESP_LOGI(TAG, "mode: %02x", mode);

		mem_write_mode_register(MEM_MODE_SEQUENTIAL);
		mode = mem_read_mode_register();
		ESP_LOGI(TAG, "mode: %02x", mode);

		vTaskDelay(1 / portTICK_PERIOD_MS);
	}

	//  never reached code
	mem_free();
	main_vspi_free();
}

void app_main() {
	main_log_configuration();
	blink_log_configuration();
	mem_log_configuration();
	fflush(stdout);

	xTaskCreate(&main_task, "main_task", 2048, NULL, 5, NULL);
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5,
			NULL);
}

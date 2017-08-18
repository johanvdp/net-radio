// The author disclaims copyright to this source code.
#include "main.h"

static const char* TAG = "main.c";

spi_device_handle_t vspi_bus_handle;
uint8_t *writeBuffer;
uint8_t *readBuffer;

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
	ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &configuration, 1));
	ESP_LOGI(TAG, "<main_vspi_initialize");
}

void main_vspi_free() {
	ESP_LOGI(TAG, ">main_vspi_free");
	ESP_ERROR_CHECK(spi_bus_free(VSPI_HOST));
	ESP_LOGI(TAG, "<main_vspi_free");
}

// transfer per byte
void main_mem_byte() {
	ESP_LOGI(TAG, ">main_mem_byte");

	mem_initialize_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_BYTE);
	uint8_t mode = mem_command_read_mode_register();
	ESP_LOGI(TAG, "mode: %02x", mode);
	mem_free();

	mem_initialize_data((spi_host_device_t) VSPI_HOST);
	uint32_t page = 0;
	uint32_t byte = 0;
	uint8_t out = 0;
	uint8_t in = 0;
	uint32_t errors = 0;
	// MEM_NUMBER_OF_PAGES takes too long, use 100 pages
	ESP_LOGI(TAG, "transfer %d pages", 100);
	for (page = 0; page < 100; page++) {
		for (byte = 0; byte < MEM_BYTES_PER_PAGE; byte++) {
			mem_data_write_byte(byte, out);
			in = mem_data_read_byte(byte);
			if (in != out) {
				errors++;
			}
			out++;
		}
		esp_task_wdt_feed();
	}
	ESP_LOGI(TAG, "errors: %d", errors);
	mem_free();

	ESP_LOGI(TAG, "<main_mem_byte");
}

// transfer per page
void main_mem_page() {
	ESP_LOGI(TAG, ">main_mem_page");

	mem_initialize_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_PAGE);
	uint8_t mode = mem_command_read_mode_register();
	ESP_LOGI(TAG, "mode: %02x", mode);
	mem_free();

	uint32_t page = 0;
	uint32_t byte = 0;
	uint8_t r = 0;
	uint8_t w = 0;
	mem_initialize_data((spi_host_device_t) VSPI_HOST);
	uint32_t errors = 0;
	ESP_LOGI(TAG, "transfer %d pages", MEM_NUMBER_OF_PAGES);
	for (page = 0; page < MEM_NUMBER_OF_PAGES; page++) {
		uint32_t address = 0;//page * MEM_BYTES_PER_PAGE;
		for (byte = 0; byte < MEM_BYTES_PER_PAGE; byte++) {
			writeBuffer[byte] = byte;
		}
		mem_data_write_page(address, writeBuffer);
		mem_data_read_page(address, readBuffer);
		for (byte = 0; byte < MEM_BYTES_PER_PAGE; byte++) {
			r = readBuffer[byte];
			w = writeBuffer[byte];
			if (w != r) {
				errors++;
			}
		}
		esp_task_wdt_feed();
	}
	ESP_LOGI(TAG, "errors: %d", errors);
	mem_free();
	ESP_LOGI(TAG, "<main_mem_page");
}

void main_buffers_initialize() {
	size_t size = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
	ESP_LOGI(TAG, "heap_caps_get_minimum_free_size: %d", size);
	writeBuffer = heap_caps_malloc(MEM_BYTES_PER_PAGE, MALLOC_CAP_DMA);
	if (writeBuffer == NULL) {
		ESP_LOGI(TAG, "heap_caps_malloc write: out of memory");
	}
	readBuffer = heap_caps_malloc(MEM_BYTES_PER_PAGE, MALLOC_CAP_DMA);
	if (readBuffer == NULL) {
		ESP_LOGI(TAG, "heap_caps_malloc read: out of memory");
	}
}

void main_buffers_free() {
	heap_caps_free(writeBuffer);
	heap_caps_free(readBuffer);
}

void main_task(void *ignore) {
	ESP_LOGI(TAG, ">main_task");
	esp_task_wdt_init();
	main_buffers_initialize();
	main_vspi_initialize();

	while (1) {
		main_mem_byte();
		main_mem_page();
	}

//	main_vspi_free();
//	main_buffers_free();
//	ESP_LOGI(TAG, "<main_task");
}

void app_main() {
	main_log_configuration();
	blink_log_configuration();
	mem_log_configuration();

	xTaskCreate(&main_task, "main_task", 2048, NULL, 5, NULL);
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);
}

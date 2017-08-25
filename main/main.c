// The author disclaims copyright to this source code.
#include "main.h"

static const char* TAG = "main.c";

#define MAIN_TEST_LENGTH 2048

spi_device_handle_t vspi_bus_handle;
uint8_t *writeBuffer;
uint8_t *readBuffer;
uint32_t bytecount;
uint32_t errorcount;

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

void main_mem_byte() {
	ESP_LOGD(TAG, ">main_mem_byte");
	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_BYTE);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	uint32_t address = 0;
	uint8_t w = 0;
	uint8_t r = 0;
	for (address = 0; address < MAIN_TEST_LENGTH; address++) {
		// write
		mem_data_write_byte(address, w);
		// read
		r = mem_data_read_byte(address);
		// check
		if (r != w) {
			errorcount++;
		}
		w++;
	}
	bytecount += MAIN_TEST_LENGTH;
	ESP_LOGI(TAG, "%d/%d", bytecount, errorcount);

	mem_end();

	ESP_LOGD(TAG, "<main_mem_byte");
}

void main_mem_page() {
	ESP_LOGD(TAG, ">main_mem_page");

	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_PAGE);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;

	for (address = 0; address < MAIN_TEST_LENGTH; address+=32) {
		// write
		w = 1;
		for (index = 0; index < MEM_BYTES_PER_PAGE; index++) {
			writeBuffer[index] = w;
			w++;
		}
		mem_data_write_page(address, writeBuffer);

		// read
		mem_data_read_page(address, readBuffer);

		// check
		errorcount = 0;
		for (index = 0; index < MEM_BYTES_PER_PAGE; index++) {
			r = readBuffer[index];
			w = writeBuffer[index];
			if (w != r) {
				errorcount++;
			}
		}

	}
	bytecount += MAIN_TEST_LENGTH;
	ESP_LOGI(TAG, "%d/%d", bytecount, errorcount);

	mem_end();

	ESP_LOGD(TAG, "<main_mem_page");
}

void main_mem_sequential() {
	ESP_LOGD(TAG, ">main_mem_sequential");

	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_SEQUENTIAL);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;

	for (address = 0; address < MAIN_TEST_LENGTH; address+=MAIN_TEST_LENGTH) {
		// write
		w = 1;
		for (index = 0; index < MAIN_TEST_LENGTH; index++) {
			writeBuffer[index] = w;
			w++;
		}
		mem_data_write(address, MAIN_TEST_LENGTH, writeBuffer);

		// read
		mem_data_read(address, MAIN_TEST_LENGTH, readBuffer);

		// check
		errorcount = 0;
		for (index = 0; index < MAIN_TEST_LENGTH; index++) {
			r = readBuffer[index];
			w = writeBuffer[index];
			if (w != r) {
				errorcount++;
			}
		}
	}
	bytecount += MAIN_TEST_LENGTH;
	ESP_LOGI(TAG, "%d/%d", bytecount, errorcount);

	mem_end();

	ESP_LOGD(TAG, "<main_mem_sequential");
}


void *main_malloc(size_t size) {
	ESP_LOGD(TAG, ">mem_malloc");
	void *buffer = heap_caps_malloc(size, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGD(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGD(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(buffer, 0, size);
	ESP_LOGD(TAG, "<mem_malloc");
	return buffer;
}

void main_buffers_malloc() {
	ESP_LOGD(TAG, ">main_buffers_malloc");
	writeBuffer = main_malloc(MAIN_TEST_LENGTH);
	readBuffer = main_malloc(MAIN_TEST_LENGTH);
	ESP_LOGD(TAG, "<main_buffers_malloc");
}

void main_buffers_free() {
	ESP_LOGD(TAG, ">main_buffers_free");
	heap_caps_free(writeBuffer);
	heap_caps_free(readBuffer);
	ESP_LOGD(TAG, "<main_buffers_free");
}

/**
 * FreeRTOS Main task is continuously running tests.
 */
void main_task(void *ignore) {
	ESP_LOGD(TAG, ">main_task");

	main_buffers_malloc();
	main_vspi_initialize();

	while (1) {

		// write and read 4096 bytes: CPU 240MHz, SPI 20MHz, 230 ms = 17808 byte/s
		//main_mem_byte();

		// write and read 4096 bytes: CPU 240MHz, SPI 20MHz, 10 ms =  40960 byte/s
		//main_mem_page();

		// write and read 4096 bytes: CPU 240MHz, SPI 20MHz, too short on log, scope 1.70 ms = 2409411 bytes/s
		main_mem_sequential();
	}

	// never reached
	//	main_vspi_free();
	//	main_buffers_free();
	//	ESP_LOGD(TAG, "<main_task");
}

/**
 * FreeRTOS Application entry point.
 */
void app_main() {
	main_log_configuration();
	blink_log_configuration();
	mem_log_configuration();

	xTaskCreate(&main_task, "main_task", 4096, NULL, 5, NULL);
	xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);
}

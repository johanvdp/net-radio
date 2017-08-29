// The author disclaims copyright to this source code.
#include "test_mem.h"

#include <string.h>
#include "spi_mem.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"

static const char* TAG = "test_mem.c";

// SPI DMA transfers are limited to 2048 bytes
#define TEST_MEM_LENGTH 2048

spi_mem_handle_t test_mem_handle;
uint8_t *test_mem_write_buffer;
uint8_t *test_mem_read_buffer;

void test_mem_log_configuration() {
	ESP_LOGD(TAG, ">test_mem_log_configuration");
	ESP_LOGI(TAG, "CONFIG_SPI_MEM_GPIO_CS: %d", CONFIG_SPI_MEM_GPIO_CS);
	ESP_LOGI(TAG, "CONFIG_SPI_MEM_SPEED_MHZ: %d", CONFIG_SPI_MEM_SPEED_MHZ);
	ESP_LOGI(TAG, "CONFIG_SPI_MEM_TOTAL_BYTES: %d", CONFIG_SPI_MEM_TOTAL_BYTES);
	ESP_LOGI(TAG, "CONFIG_SPI_MEM_NUMBER_OF_PAGES: %d", CONFIG_SPI_MEM_NUMBER_OF_PAGES);
	ESP_LOGI(TAG, "CONFIG_SPI_MEM_BYTES_PER_PAGE: %d", CONFIG_SPI_MEM_BYTES_PER_PAGE);
	ESP_LOGD(TAG, "<test_mem_log_configuration");
}

void test_mem_byte() {
	ESP_LOGD(TAG, ">test_mem_byte");

	spi_mem_write_mode_register(test_mem_handle, SPI_MEM_MODE_BYTE);
	spi_mem_read_mode_register(test_mem_handle);

	uint32_t address = 0;
	uint8_t w = 0;
	uint8_t r = 0;
	uint32_t errorcount = 0;
	for (address = 0; address < TEST_MEM_LENGTH; address++) {
		// write
		spi_mem_write_byte(test_mem_handle, address, w);
		// read
		r = spi_mem_read_byte(test_mem_handle, address);
		// check
		if (r != w) {
			errorcount++;
		}
		w++;
	}
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_byte FAIL %d/%d", TEST_MEM_LENGTH, errorcount);
	} else {
		ESP_LOGI(TAG, "test_mem_byte OK");
	}

	ESP_LOGD(TAG, "<test_mem_byte");
}

void test_mem_page() {
	ESP_LOGD(TAG, ">test_mem_page");

	spi_mem_write_mode_register(test_mem_handle, SPI_MEM_MODE_PAGE);
	spi_mem_read_mode_register(test_mem_handle);

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t bytecount = 0;
	uint32_t errorcount = 0;

	for (address = 0; address < TEST_MEM_LENGTH; address +=
	CONFIG_SPI_MEM_BYTES_PER_PAGE) {
		// write
		for (index = 0; index < CONFIG_SPI_MEM_BYTES_PER_PAGE; index++) {
			test_mem_write_buffer[index] = w;
			w++;
		}
		spi_mem_write_page(test_mem_handle, address, test_mem_write_buffer);

		// read
		spi_mem_read_page(test_mem_handle, address, test_mem_read_buffer);

		// check
		errorcount = 0;
		for (index = 0; index < CONFIG_SPI_MEM_BYTES_PER_PAGE; index++) {
			r = test_mem_read_buffer[index];
			w = test_mem_write_buffer[index];
			if (w != r) {
				errorcount++;
			}
		}
		bytecount += CONFIG_SPI_MEM_BYTES_PER_PAGE;
	}
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_page FAIL %d/%d", bytecount, errorcount);
	} else {
		ESP_LOGI(TAG, "test_mem_page OK");
	}

	ESP_LOGD(TAG, "<test_mem_page");
}

void test_mem_sequential() {
	ESP_LOGD(TAG, ">test_mem_sequential");

	spi_mem_write_mode_register(test_mem_handle, SPI_MEM_MODE_SEQUENTIAL);
	spi_mem_read_mode_register(test_mem_handle);

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t errorcount = 0;

	// write
	for (index = 0; index < TEST_MEM_LENGTH; index++) {
		test_mem_write_buffer[index] = w;
		w++;
	}
	spi_mem_write(test_mem_handle, address, TEST_MEM_LENGTH, test_mem_write_buffer);

	// read
	spi_mem_read(test_mem_handle, address, TEST_MEM_LENGTH, test_mem_read_buffer);

	// check
	for (index = 0; index < TEST_MEM_LENGTH; index++) {
		r = test_mem_read_buffer[index];
		w = test_mem_write_buffer[index];
		if (w != r) {
			errorcount++;
		}
	}
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_sequential FAIL %d/%d", TEST_MEM_LENGTH, errorcount);
	} else {
		ESP_LOGI(TAG, "test_mem_sequential OK");
	}

	ESP_LOGD(TAG, "<test_mem_sequential");
}

void *test_mem_malloc(size_t size) {
	ESP_LOGD(TAG, ">test_mem_malloc");
	void *buffer = heap_caps_malloc(size, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGI(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(buffer, 0, size);
	ESP_LOGD(TAG, "<test_mem_malloc");
	return buffer;
}

void test_mem_buffers_malloc() {
	ESP_LOGD(TAG, ">test_mem_buffers_malloc");
	test_mem_write_buffer = test_mem_malloc(TEST_MEM_LENGTH);
	test_mem_read_buffer = test_mem_malloc(TEST_MEM_LENGTH);
	ESP_LOGD(TAG, "<test_mem_buffers_malloc");
}

void test_mem_buffers_free() {
	ESP_LOGD(TAG, ">test_mem_buffers_free");
	heap_caps_free(test_mem_write_buffer);
	heap_caps_free(test_mem_read_buffer);
	ESP_LOGD(TAG, "<test_mem_buffers_free");
}

/**
 * FreeRTOS MEM test task runs once.
 */
void test_mem_task(void *ignore) {
	ESP_LOGD(TAG, ">test_mem_task");
	test_mem_log_configuration();

	spi_mem_config_t configuration;
	memset(&configuration, 0, sizeof(spi_mem_config_t));
	configuration.host = (spi_host_device_t) VSPI_HOST;
	configuration.clock_speed_hz = CONFIG_SPI_MEM_SPEED_MHZ * 1000000;
	configuration.spics_io_num = CONFIG_SPI_MEM_GPIO_CS;
	configuration.total_bytes = CONFIG_SPI_MEM_TOTAL_BYTES;
	configuration.number_of_pages = CONFIG_SPI_MEM_NUMBER_OF_PAGES;
	configuration.number_of_bytes_page = CONFIG_SPI_MEM_BYTES_PER_PAGE;
	spi_mem_begin(configuration, &test_mem_handle);

	ESP_LOGD(TAG, "test_mem_spi_mem_handle=%p", test_mem_handle);

	test_mem_buffers_malloc();

	while (1) {
		// write and read TEST_MEM_LENGTH bytes per byte
		test_mem_byte();

		// write and read TEST_MEM_LENGTH bytes per MEM_BYTES_PER_PAGE
		test_mem_page();

		// write and read TEST_MEM_LENGTH bytes at once
		test_mem_sequential();
	}

	// never reached
	//spi_mem_end(test_mem_spi_mem_handle);
	//test_mem_buffers_free();
	//ESP_LOGI(TAG, "<test_mem_task");
	//vTaskDelete(NULL);
}


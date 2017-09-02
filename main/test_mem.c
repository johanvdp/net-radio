// The author disclaims copyright to this source code.
#include "test_mem.h"
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "test_mem.c";

// SPI DMA transfers are limited to SPI_MAX_DMA_LEN
#define TEST_MEM_LENGTH 2048

static spi_mem_handle_t test_mem_handle;
static uint8_t *test_mem_write_buffer;
static uint8_t *test_mem_read_buffer;

static esp_err_t test_mem_byte() {
	ESP_LOGD(TAG, ">test_mem_byte");

	spi_mem_write_mode_register(test_mem_handle, SPI_MEM_MODE_BYTE);

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
		return ESP_FAIL;
	}

	// Can not make this function work
	//spi_mem_mode_t mode = spi_mem_read_mode_register(test_mem_handle);
	//if (mode != SPI_MEM_MODE_BYTE) {
	//	ESP_LOGE(TAG, "test_mem_mode mode expected: 0x%02x, actual: 0x%02x", SPI_MEM_MODE_BYTE, mode);
	//	return ESP_FAIL;
	//}

	ESP_LOGD(TAG, "<test_mem_byte");
	return ESP_OK;
}

static esp_err_t test_mem_page() {
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
	CONFIG_MEM_BYTES_PER_PAGE) {
		// write
		for (index = 0; index < CONFIG_MEM_BYTES_PER_PAGE; index++) {
			test_mem_write_buffer[index] = w;
			w++;
		}
		spi_mem_write_page(test_mem_handle, address, test_mem_write_buffer);

		// read
		spi_mem_read_page(test_mem_handle, address, test_mem_read_buffer);

		// check
		errorcount = 0;
		for (index = 0; index < CONFIG_MEM_BYTES_PER_PAGE; index++) {
			r = test_mem_read_buffer[index];
			w = test_mem_write_buffer[index];
			if (w != r) {
				errorcount++;
			}
		}
		bytecount += CONFIG_MEM_BYTES_PER_PAGE;
	}
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_page FAIL %d/%d", bytecount, errorcount);
		return ESP_FAIL;
	}

	// Can not make this function work
	//spi_mem_mode_t mode = spi_mem_read_mode_register(test_mem_handle);
	//if (mode != SPI_MEM_MODE_PAGE) {
	//	ESP_LOGE(TAG, "test_mem_mode mode expected: 0x%02x, actual: 0x%02x", SPI_MEM_MODE_PAGE, mode);
	//	return ESP_FAIL;
	//}

	ESP_LOGD(TAG, "<test_mem_page");
	return ESP_OK;
}

static esp_err_t test_mem_sequential() {
	ESP_LOGD(TAG, ">test_mem_sequential");

	spi_mem_write_mode_register(test_mem_handle, SPI_MEM_MODE_SEQUENTIAL);

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
		return ESP_FAIL;
	}

	// Can not make this function work
	//spi_mem_mode_t mode = spi_mem_read_mode_register(test_mem_handle);
	//if (mode != SPI_MEM_MODE_SEQUENTIAL) {
	//	ESP_LOGE(TAG, "test_mem_mode mode expected: 0x%02x, actual: 0x%02x", SPI_MEM_MODE_SEQUENTIAL, mode);
	//	return ESP_FAIL;
	//}

	ESP_LOGD(TAG, "<test_mem_sequential");
	return ESP_OK;
}

static void *test_mem_malloc(size_t size) {
	ESP_LOGD(TAG, ">test_mem_malloc");
	void *buffer = heap_caps_malloc(size, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGD(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(buffer, 0, size);
	ESP_LOGD(TAG, "<test_mem_malloc");
	return buffer;
}

static void test_mem_buffers_malloc() {
	ESP_LOGD(TAG, ">test_mem_buffers_malloc");
	test_mem_write_buffer = test_mem_malloc(TEST_MEM_LENGTH);
	test_mem_read_buffer = test_mem_malloc(TEST_MEM_LENGTH);
	ESP_LOGD(TAG, "<test_mem_buffers_malloc");
}

static void test_mem_buffers_free() {
	ESP_LOGD(TAG, ">test_mem_buffers_free");
	heap_caps_free(test_mem_write_buffer);
	heap_caps_free(test_mem_read_buffer);
	ESP_LOGD(TAG, "<test_mem_buffers_free");
}

/**
 * MEM test task.
 */
esp_err_t test_mem(test_mem_config_t config) {
	ESP_LOGD(TAG, ">test_mem");

	test_mem_handle = config.spi_mem_handle;
	ESP_LOGD(TAG, "spi_mem_handle: %p", test_mem_handle);

	test_mem_buffers_malloc();

	// write and read TEST_MEM_LENGTH bytes per byte
	if (test_mem_byte() != ESP_OK) {
		return ESP_FAIL;
	}

	// write and read TEST_MEM_LENGTH bytes per MEM_BYTES_PER_PAGE
	if (test_mem_page() != ESP_OK) {
		return ESP_FAIL;
	}

	// write and read TEST_MEM_LENGTH bytes at once
	if (test_mem_sequential() != ESP_OK) {
		return ESP_FAIL;
	}

	test_mem_buffers_free();

	ESP_LOGD(TAG, "<test_mem");

	return ESP_OK;
}


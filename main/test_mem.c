// The author disclaims copyright to this source code.
#include "test_mem.h"

static const char* TAG = "test_mem.c";

// SPI DMA transfers are limited to 2048 bytes
#define TEST_MEM_LENGTH 2048

spi_device_handle_t vspi_bus_handle;
uint8_t *writeBuffer;
uint8_t *readBuffer;

void test_mem_log_configuration() {
	ESP_LOGD(TAG, ">test_mem_log_configuration");
	// ESP_LOGI(TAG, "CONFIG_: %d", CONFIG_);
	ESP_LOGD(TAG, "<test_mem_log_configuration");
}

void test_mem_byte() {
	ESP_LOGD(TAG, ">test_mem_byte");
	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_BYTE);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	uint32_t address = 0;
	uint8_t w = 0;
	uint8_t r = 0;
	uint32_t errorcount = 0;
	for (address = 0; address < TEST_MEM_LENGTH; address++) {
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
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_byte FAIL %d/%d", TEST_MEM_LENGTH, errorcount);
	} else {
		ESP_LOGI(TAG, "test_mem_byte OK");
	}

	mem_end();

	ESP_LOGD(TAG, "<test_mem_byte");
}

void test_mem_page() {
	ESP_LOGD(TAG, ">test_mem_page");

	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_PAGE);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t bytecount = 0;
	uint32_t errorcount = 0;

	for (address = 0; address < TEST_MEM_LENGTH; address +=
	MEM_BYTES_PER_PAGE) {
		// write
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
		bytecount += MEM_BYTES_PER_PAGE;
	}
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_page FAIL %d/%d", bytecount, errorcount);
	} else {
		ESP_LOGI(TAG, "test_mem_page OK");
	}

	mem_end();

	ESP_LOGD(TAG, "<test_mem_page");
}

void test_mem_sequential() {
	ESP_LOGD(TAG, ">test_mem_sequential");

	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_SEQUENTIAL);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	uint8_t r = 0;
	uint8_t w = 0;
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t errorcount = 0;

	// write
	for (index = 0; index < TEST_MEM_LENGTH; index++) {
		writeBuffer[index] = w;
		w++;
	}
	mem_data_write(address, TEST_MEM_LENGTH, writeBuffer);

	// read
	mem_data_read(address, TEST_MEM_LENGTH, readBuffer);

	// check
	for (index = 0; index < TEST_MEM_LENGTH; index++) {
		r = readBuffer[index];
		w = writeBuffer[index];
		if (w != r) {
			errorcount++;
		}
	}
	if (errorcount > 0) {
		ESP_LOGE(TAG, "test_mem_sequential FAIL %d/%d", TEST_MEM_LENGTH, errorcount);
	} else {
		ESP_LOGI(TAG, "test_mem_sequential OK");
	}

	mem_end();

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
	writeBuffer = test_mem_malloc(TEST_MEM_LENGTH);
	readBuffer = test_mem_malloc(TEST_MEM_LENGTH);
	ESP_LOGD(TAG, "<test_mem_buffers_malloc");
}

void test_mem_buffers_free() {
	ESP_LOGD(TAG, ">test_mem_buffers_free");
	heap_caps_free(writeBuffer);
	heap_caps_free(readBuffer);
	ESP_LOGD(TAG, "<test_mem_buffers_free");
}

/**
 * FreeRTOS MEM test task runs once.
 */
void test_mem_task(void *ignore) {
	ESP_LOGI(TAG, ">test_mem_task");

	test_mem_buffers_malloc();

	// write and read TEST_MEM_LENGTH bytes per byte
	test_mem_byte();

	// write and read TEST_MEM_LENGTH bytes per MEM_BYTES_PER_PAGE
	test_mem_page();

	// write and read TEST_MEM_LENGTH bytes at once
	test_mem_sequential();

	test_mem_buffers_free();

	ESP_LOGI(TAG, "<test_mem_task");

	vTaskDelete(NULL);
}


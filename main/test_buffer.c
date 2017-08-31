// The author disclaims copyright to this source code.
#include "test_buffer.h"

#include <string.h>
#include "buffer.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "tinymt32.h"

static const char* TAG = "test_buffer.c";

// SPI DMA transfers are limited to SPI_MAX_DMA_LEN
#define DMA_MAX_LENGTH 2048

buffer_handle_t test_buffer_handle;
uint8_t *test_buffer_data;
tinymt32_t test_buffer_tinymt;

void *test_buffer_malloc(size_t size) {
	ESP_LOGD(TAG, ">test_buffer_malloc");
	void *buffer = heap_caps_malloc(size, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGI(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(buffer, 0, size);
	ESP_LOGD(TAG, "<test_buffer_malloc");
	return buffer;
}

void test_buffer_data_malloc() {
	ESP_LOGD(TAG, ">test_buffer_data_malloc");
	test_buffer_data = test_buffer_malloc(DMA_MAX_LENGTH);
	ESP_LOGD(TAG, "<test_buffer_data_malloc");
}

void test_buffer_data_free() {
	ESP_LOGD(TAG, ">test_buffer_data_free");
	heap_caps_free(test_buffer_data);
	test_buffer_data = NULL;
	ESP_LOGD(TAG, "<test_buffer_data_free");
}

void test_buffer_reset() {
	buffer_reset(test_buffer_handle);

	uint32_t available_actual = buffer_available(test_buffer_handle);
	uint32_t available_expected = 0;
	if (available_actual != available_expected) {
		ESP_LOGE(TAG, "buffer_available expected: %d, actual: %d", available_expected, available_actual);
		return;
	}

	uint32_t free_actual = buffer_free(test_buffer_handle);
	uint32_t free_expected = test_buffer_handle->size;
	if (free_actual != free_expected) {
		ESP_LOGE(TAG, "buffer_free expected: %d, actual: %d", free_expected, free_actual);
		return;
	}
}

void test_buffer_tinymt_init() {
	ESP_LOGD(TAG, ">test_buffer_tinymt_init");
	test_buffer_tinymt.mat1 = 0x8f7011ee;
	test_buffer_tinymt.mat2 = 0xfc78ff1f;
	test_buffer_tinymt.tmat = 0x3793fdff;
	tinymt32_init(&test_buffer_tinymt, 1);
	ESP_LOGD(TAG, "<test_buffer_tinymt_init");
}

void test_buffer_check_size(int available_expected) {
	uint32_t available_actual = buffer_available(test_buffer_handle);
	if (available_actual != available_expected) {
		buffer_log(test_buffer_handle);
		ESP_LOGE(TAG, "buffer_available expected: %d, actual: %d", available_expected, available_actual);
		return;
	}

	uint32_t free_actual = buffer_free(test_buffer_handle);
	uint32_t free_expected = (test_buffer_handle->size - available_expected);
	if (free_actual != free_expected) {
		buffer_log(test_buffer_handle);
		ESP_LOGE(TAG, "buffer_free expected: %d, actual: %d", free_expected, free_actual);
		return;
	}
}

void test_buffer_push(uint32_t size) {
	ESP_LOGI(TAG, "push %d", size);
	int remaining = size;
	while (remaining > 0) {
		// limit transfer length
		int max = remaining > DMA_MAX_LENGTH ? DMA_MAX_LENGTH : remaining;
		for (int i = 0; i < max; i++) {
			test_buffer_data[i] = (uint8_t) tinymt32_generate_uint32(&test_buffer_tinymt);
		}
		buffer_push(test_buffer_handle, test_buffer_data, max);
		remaining -= max;
	}
}

void test_buffer_check_value(int size) {
	for (int i = 0; i < size; i++) {
		uint8_t value_actual = test_buffer_data[i];
		uint8_t value_expected = tinymt32_generate_uint32(&test_buffer_tinymt);
		if (value_actual != value_expected) {
			buffer_log(test_buffer_handle);
			ESP_LOGE(TAG, "value expected: %d, actual: %d", value_expected, value_actual);
			return;
		}
	}
}

void test_buffer_pull(uint32_t size) {
	ESP_LOGI(TAG, "pull %d", size);
	uint32_t remaining = size;
	while (remaining > 0) {
		// limit transfer length
		int max = remaining > DMA_MAX_LENGTH ? DMA_MAX_LENGTH : remaining;
		buffer_pull(test_buffer_handle, max, test_buffer_data);
		test_buffer_check_value(max);
		remaining -= max;
	}
}

void test_buffer_push_pull() {
	// prepare
	buffer_reset(test_buffer_handle);

	// push 20: expect available=20 and free=(test_buffer_handle->size - 20)
	test_buffer_tinymt_init();
	test_buffer_push(20);
	test_buffer_check_size(20);

	// pull 10: expect available=10, free=(test_buffer_handle->size - 10), and data matches
	test_buffer_tinymt_init();
	test_buffer_pull(10);
	test_buffer_check_size(10);

	// push (test_buffer_handle->size - 15): expect available=(test_buffer_handle->size - 5) and free=5
	// this will demonstrate wrap around top
	test_buffer_tinymt_init();
	test_buffer_push(test_buffer_handle->size - 15);
	test_buffer_check_size(test_buffer_handle->size - 5);

	// first 10 where already pulled, but need to generate 'random' value sequence
	test_buffer_tinymt_init();
	ESP_LOGI(TAG, "skip 10");
	for (int i = 0; i < 10; i++) {
		tinymt32_generate_uint32(&test_buffer_tinymt);
	}

	// pull 10
	ESP_LOGI(TAG, "pull 10");
	buffer_pull(test_buffer_handle, 10, test_buffer_data);
	test_buffer_check_size(test_buffer_handle->size - 15);
	test_buffer_check_value(10);

	// pull remainder
	test_buffer_tinymt_init();
	test_buffer_pull(test_buffer_handle->size - 15);
	test_buffer_check_size(0);
}

/**
 * FreeRTOS Buffer test task.
 */
void test_buffer_task(void *pvParameters) {
	ESP_LOGI(TAG, ">test_buffer_task");

	test_buffer_config_t *config = (test_buffer_config_t *) pvParameters;
	test_buffer_handle = config->buffer_handle;
	ESP_LOGD(TAG, "test_buffer_handle: %p", test_buffer_handle);

	test_buffer_data_malloc();

	while (1) {
		test_buffer_reset();

		test_buffer_push_pull();

		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}


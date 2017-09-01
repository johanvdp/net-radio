// The author disclaims copyright to this source code.
#include "reader.h"
#include <string.h>
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "hello_mp3.c"

static const char* TAG = "reader.c";

// SPI DMA transfers are limited to SPI_MAX_DMA_LEN
#define DMA_MAX_LENGTH 2048

buffer_handle_t reader_buffer_handle;
uint8_t *reader_data;

void *reader_malloc(size_t size) {
	ESP_LOGD(TAG, ">reader_malloc");
	void *buffer = heap_caps_malloc(size, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGI(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(buffer, 0, size);
	ESP_LOGD(TAG, "<reader_malloc");
	return buffer;
}

void reader_data_malloc() {
	ESP_LOGD(TAG, ">reader_data_malloc");
	reader_data = reader_malloc(DMA_MAX_LENGTH);
	ESP_LOGD(TAG, "<reader_data_malloc");
}

void reader_data_free() {
	ESP_LOGD(TAG, ">reader_data_free");
	heap_caps_free(reader_data);
	reader_data = NULL;
	ESP_LOGD(TAG, "<reader_data_free");
}

void reader_push_hello() {
	ESP_LOGD(TAG, ">reader_push_hello");
	const uint8_t *p = &HELLO_MP3[0];
	uint32_t remainder = sizeof(HELLO_MP3);
	while (remainder > 0) {
		// limit to transfer size
		uint32_t max = (remainder > DMA_MAX_LENGTH ? DMA_MAX_LENGTH : remainder);
		// limit to available space
		uint32_t free = buffer_free(reader_buffer_handle);
		max = max > free ? free : max;
		if (max > 0) {
			// push into buffer
			memcpy(reader_data, p, max);
			ESP_LOGV(TAG, "buffer_push %p %p %d", reader_buffer_handle, reader_data, max);
			buffer_push(reader_buffer_handle, reader_data, max);
			p += max;
			remainder -= max;
		} else {
			// wait for available space
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
	}
	ESP_LOGD(TAG, "<reader_push_hello");
}
/**
 * FreeRTOS Reader task.
 */
void reader_task(void *pvParameters) {
	ESP_LOGI(TAG, ">reader_task");

	reader_config_t *config = (reader_config_t *) pvParameters;
	reader_buffer_handle = config->buffer_handle;
	ESP_LOGD(TAG, "reader_buffer_handle: %p", reader_buffer_handle);

	reader_data_malloc();

	while (1) {
		reader_push_hello();
		taskYIELD();
	}

	// never reached
	//reader_data_free();
	//ESP_LOGI(TAG, "<reader_task");
	//vTaskDelete(NULL);
}


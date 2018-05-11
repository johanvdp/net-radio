// The author disclaims copyright to this source code.
#include "hello.h"
#include <string.h>
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "hello_mp3.c"

static const char* TAG = "hello";

// SPI DMA transfers are limited to SPI_MAX_DMA_LEN
#define DMA_MAX_LENGTH 2048

static buffer_handle_t hello_buffer_handle;
static uint8_t *hello_data;

static void *hello_malloc(size_t size) {
	ESP_LOGD(TAG, ">hello_malloc");
	void *buffer = heap_caps_malloc(size, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGD(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(buffer, 0, size);
	ESP_LOGD(TAG, "<hello_malloc");
	return buffer;
}

static void hello_data_malloc() {
	ESP_LOGD(TAG, ">hello_data_malloc");
	hello_data = hello_malloc(DMA_MAX_LENGTH);
	ESP_LOGD(TAG, "<hello_data_malloc");
}

static void hello_push_hello() {
	ESP_LOGD(TAG, ">hello_push_hello");
	const uint8_t *p = &HELLO_MP3[0];
	uint32_t remainder = sizeof(HELLO_MP3);
	while (remainder > 0) {
		// limit to transfer size
		uint32_t transfer = (remainder > DMA_MAX_LENGTH ? DMA_MAX_LENGTH : remainder);
		// limit to available space
		uint32_t free = buffer_free(hello_buffer_handle);
		transfer = transfer > free ? free : transfer;
		if (transfer > 0) {
			// push into buffer
			memcpy(hello_data, p, transfer);
			ESP_LOGV(TAG, "buffer_push %p %p %d", hello_buffer_handle, hello_data, transfer);
			buffer_push(hello_buffer_handle, hello_data, transfer);
			p += transfer;
			remainder -= transfer;
		} else {
			// wait for available space
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
	}
	ESP_LOGD(TAG, "<hello_push_hello");
}

/**
 * FreeRTOS Hello task.
 */
void hello_task(void *pvParameters) {
	ESP_LOGI(TAG, ">hello_task");

	hello_config_t *config = (hello_config_t *) pvParameters;
	hello_buffer_handle = config->buffer_handle;
	ESP_LOGD(TAG, "hello_buffer_handle: %p", hello_buffer_handle);

	hello_data_malloc();

	while (1) {
		hello_push_hello();
		// not too many
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	// should never be reached
}


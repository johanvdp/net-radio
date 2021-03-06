// The author disclaims copyright to this source code.
#include "player.h"
#include <string.h>
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "player";

static vs1053_handle_t player_vs1053_handle;
static buffer_handle_t player_buffer_handle;
static uint8_t *player_data;

static void player_data_malloc() {
	ESP_LOGD(TAG, ">player_data_malloc");
	player_data = heap_caps_malloc(VS1053_MAX_DATA_SIZE, MALLOC_CAP_DMA);
	if (player_data == NULL) {
		ESP_LOGE(TAG, "heap_caps_malloc: out of memory");
		size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
		ESP_LOGD(TAG, "heap_caps_get_minimum_free_size: %d", available);
	}
	memset(player_data, 0, VS1053_MAX_DATA_SIZE);
	ESP_LOGD(TAG, "<player_data_malloc");
}

/**
 * FreeRTOS Player task.
 */
void player_task(void *pvParameters) {
	ESP_LOGI(TAG, ">player_task");
	ESP_LOGD(TAG, "CONFIG_DSP_GPIO_XCS: %d", CONFIG_DSP_GPIO_XCS);
	ESP_LOGD(TAG, "CONFIG_DSP_GPIO_XDCS: %d", CONFIG_DSP_GPIO_XDCS);
	ESP_LOGD(TAG, "CONFIG_DSP_GPIO_DREQ: %d", CONFIG_DSP_GPIO_DREQ);
	ESP_LOGD(TAG, "CONFIG_DSP_SPI_SPEED_START_KHZ: %d", CONFIG_DSP_SPI_SPEED_START_KHZ);
	ESP_LOGD(TAG, "CONFIG_DSP_SPI_SPEED_KHZ: %d", CONFIG_DSP_SPI_SPEED_KHZ);

	player_data_malloc();

	player_config_t *config = (player_config_t *)pvParameters;
	player_buffer_handle = config->buffer_handle;
	player_vs1053_handle = config->vs1053_handle;
	ESP_LOGD(TAG, "player_buffer_handle: %p", player_buffer_handle);
	ESP_LOGD(TAG, "player_vs1053_handle: %p", player_vs1053_handle);

	while (1) {
		// check buffer (polling for now)
		uint32_t available = buffer_available(player_buffer_handle);
		if (available > 0) {
			// read buffer
			uint32_t length = available > VS1053_MAX_DATA_SIZE ? VS1053_MAX_DATA_SIZE : available;
			ESP_LOGV(TAG, "buffer_pull %p %d %p", player_buffer_handle, length, player_data);
			buffer_pull(player_buffer_handle, length, player_data);
			// write decoder
			ESP_LOGV(TAG, "vs1053_decode %p %p %d", player_vs1053_handle, player_data, length);
			vs1053_decode(player_vs1053_handle, player_data, length);
		} else {
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
	}
	// should never be reached
}

// The author disclaims copyright to this source code.
#include "statistics.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "statistics.c";

buffer_handle_t statistics_buffer_handle;
uint32_t statistics_previous_pull_bytes;
uint32_t statistics_previous_push_bytes;
uint32_t statistics_previous_pull_count;
uint32_t statistics_previous_push_count;

void statistics_task(void *pvParameters) {
	ESP_LOGD(TAG, ">statistics_task");

	statistics_config_t *config = (statistics_config_t *) pvParameters;
	statistics_buffer_handle = config->buffer_handle;
	ESP_LOGD(TAG, "statistics_buffer_handle: %p", statistics_buffer_handle);

	// loop forever
	while (1) {

		uint32_t pull_bytes = statistics_buffer_handle->pull_bytes;
		uint32_t push_bytes = statistics_buffer_handle->push_bytes;
		uint32_t pull_count = statistics_buffer_handle->pull_count;
		uint32_t push_count = statistics_buffer_handle->push_count;

		uint32_t pull_bytes_per_second = (pull_bytes - statistics_previous_pull_bytes);
		uint32_t push_bytes_per_second = (push_bytes - statistics_previous_push_bytes);
		uint32_t pull_count_per_second = (pull_count - statistics_previous_pull_count);
		uint32_t push_count_per_second = (push_count - statistics_previous_push_count);

		uint32_t available = statistics_buffer_handle->write_addr - statistics_buffer_handle->read_addr;
		uint32_t percentage = 100 * available / statistics_buffer_handle->size;

		statistics_previous_pull_bytes = pull_bytes;
		statistics_previous_push_bytes = push_bytes;
		statistics_previous_pull_count = pull_count;
		statistics_previous_push_count = push_count;

		ESP_LOGI(TAG, "push_count: %10u %10u", push_count, push_count_per_second);
		ESP_LOGI(TAG, "push_bytes: %10u %10u", push_bytes, push_bytes_per_second);

		ESP_LOGI(TAG, "pull_count: %10u %10u", pull_count, pull_count_per_second);
		ESP_LOGI(TAG, "pull_bytes: %10u %10u", pull_bytes, pull_bytes_per_second);

		ESP_LOGI(TAG, "usage: %10u %10u", available, percentage);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	// never reached
	// ESP_LOGD(TAG, "<blink_task");
}


// The author disclaims copyright to this source code.
#include "buffer.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "buffer.c";

void buffer_log_config(buffer_config_t config) {
	ESP_LOGD(TAG, ">buffer_log_config");
	ESP_LOGI(TAG, "spi_mem_handle: %p", config.spi_mem_handle);
	ESP_LOGI(TAG, "size: %d", config.size);
	ESP_LOGD(TAG, "<buffer_log_config");
}

void buffer_log(buffer_handle_t handle) {
	assert(xSemaphoreTake(handle->mutex, portMAX_DELAY) == pdTRUE);
	ESP_LOGD(TAG, ">buffer_log");
	ESP_LOGI(TAG, "handle: %p", handle);
	ESP_LOGI(TAG, "spi_mem_handle: %p", handle->spi_mem_handle);
	ESP_LOGI(TAG, "size: %d", handle->size);
	ESP_LOGI(TAG, "mask: 0x%04x", handle->mask);
	ESP_LOGI(TAG, "buffer_read_addr: %d", handle->read_addr);
	ESP_LOGI(TAG, "buffer_write_addr: %d", handle->write_addr);
	ESP_LOGI(TAG, "mutex: %p", handle->mutex);
	ESP_LOGI(TAG, "pull_bytes: %u", handle->pull_bytes);
	ESP_LOGI(TAG, "push_bytes: %u", handle->push_bytes);
	ESP_LOGI(TAG, "pull_count: %u", handle->pull_count);
	ESP_LOGI(TAG, "push_count: %u", handle->push_count);
	ESP_LOGD(TAG, "<buffer_log");
	assert(xSemaphoreGive(handle->mutex) == pdTRUE);
}

uint32_t buffer_available(buffer_handle_t handle) {
	ESP_LOGV(TAG, ">buffer_available");
	assert(xSemaphoreTake(handle->mutex, portMAX_DELAY) == pdTRUE);
	uint32_t available = handle->write_addr - handle->read_addr;
	assert(xSemaphoreGive(handle->mutex) == pdTRUE);
	ESP_LOGV(TAG, "<buffer_available");
	return available;
}

uint32_t buffer_free(buffer_handle_t handle) {
	ESP_LOGV(TAG, ">buffer_free");
	assert(xSemaphoreTake(handle->mutex, portMAX_DELAY) == pdTRUE);
	uint32_t free = handle->size - (handle->write_addr - handle->read_addr);
	assert(xSemaphoreGive(handle->mutex) == pdTRUE);
	ESP_LOGV(TAG, "<buffer_free");
	return free;
}

void buffer_push(buffer_handle_t handle, uint8_t *data, uint32_t length) {
	ESP_LOGV(TAG, ">buffer_push");
	assert(xSemaphoreTake(handle->mutex, portMAX_DELAY) == pdTRUE);
	assert(length <= (handle->size - (handle->write_addr - handle->read_addr)));
	uint32_t current = handle->write_addr & handle->mask;
	spi_mem_write(handle->spi_mem_handle, current, length, data);
	handle->write_addr = (handle->write_addr + length);
	handle->push_bytes += length;
	handle->push_count++;
	assert(xSemaphoreGive(handle->mutex) == pdTRUE);
	ESP_LOGV(TAG, "<buffer_push");
}

void buffer_pull(buffer_handle_t handle, uint32_t length, uint8_t *data) {
	ESP_LOGV(TAG, ">buffer_pull");
	assert(xSemaphoreTake(handle->mutex, portMAX_DELAY) == pdTRUE);
	assert(length <= (handle->write_addr - handle->read_addr));
	uint32_t current = (handle->read_addr) & handle->mask;
	spi_mem_read(handle->spi_mem_handle, current, length, data);
	handle->read_addr = (handle->read_addr + length);
	handle->pull_bytes += length;
	handle->pull_count++;
	assert(xSemaphoreGive(handle->mutex) == pdTRUE);
	ESP_LOGV(TAG, "<buffer_pull");
}

bool buffer_is_power_of_two(uint32_t size) {
	return (size != 0) && ((size & (size - 1)) == 0);
}

void buffer_begin(buffer_config_t config, buffer_handle_t *handle) {
	ESP_LOGD(TAG, ">buffer_begin");

	assert(buffer_is_power_of_two(config.size));

	buffer_handle_t buffer_handle = malloc(sizeof(struct buffer_t));
	buffer_handle->spi_mem_handle = config.spi_mem_handle;
	buffer_handle->size = config.size;
	buffer_handle->mask = config.size - 1;
	buffer_handle->read_addr = 0;
	buffer_handle->write_addr = 0;
	buffer_handle->push_bytes = 0;
	buffer_handle->pull_bytes = 0;
	buffer_handle->push_count = 0;
	buffer_handle->pull_count = 0;
	buffer_handle->mutex = xSemaphoreCreateMutex();
	assert(buffer_handle->mutex != NULL);

	// in sequential mode memory addressing will wrap like the buffer does
	spi_mem_write_mode_register(buffer_handle->spi_mem_handle, SPI_MEM_MODE_SEQUENTIAL);
	spi_mem_read_mode_register(buffer_handle->spi_mem_handle);

	*handle = buffer_handle;

	ESP_LOGD(TAG, "<buffer_begin");
}

void buffer_end(buffer_handle_t handle) {
	ESP_LOGD(TAG, ">buffer_end");
	spi_mem_end(handle->spi_mem_handle);
	handle->spi_mem_handle = NULL;
	handle->read_addr = 0;
	handle->write_addr = 0;
	vSemaphoreDelete(handle->mutex);
	handle->mutex = NULL;
	handle->pull_bytes = 0;
	handle->push_bytes = 0;
	free(handle);
	ESP_LOGD(TAG, "<buffer_end");
}

void buffer_reset(buffer_handle_t handle) {
	ESP_LOGD(TAG, ">buffer_reset");
	assert(xSemaphoreTake(handle->mutex, portMAX_DELAY) == pdTRUE);
	handle->read_addr = 0;
	handle->write_addr = 0;
	handle->push_bytes = 0;
	handle->pull_bytes = 0;
	handle->push_count = 0;
	handle->pull_count = 0;
	assert(xSemaphoreGive(handle->mutex) == pdTRUE);
	ESP_LOGD(TAG, "<buffer_reset");
}

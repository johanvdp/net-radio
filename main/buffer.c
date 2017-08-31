// The author disclaims copyright to this source code.
#include "buffer.h"

static const char* TAG = "buffer.c";

void buffer_log_config(buffer_config_t config) {
	ESP_LOGD(TAG, ">buffer_log_config");
	ESP_LOGI(TAG, "spi_mem_handle: %p", config.spi_mem_handle);
	ESP_LOGI(TAG, "size: %d", config.size);
	ESP_LOGD(TAG, "<buffer_log_config");
}

void buffer_log(buffer_handle_t handle) {
	ESP_LOGD(TAG, ">buffer_log");
	ESP_LOGI(TAG, "handle: %p", handle);
	ESP_LOGI(TAG, "spi_mem_handle: %p", handle->spi_mem_handle);
	ESP_LOGI(TAG, "size: %d", handle->size);
	ESP_LOGI(TAG, "mask: 0x%04x", handle->mask);
	ESP_LOGI(TAG, "buffer_read_addr: %d", handle->buffer_read_addr);
	ESP_LOGI(TAG, "buffer_write_addr: %d", handle->buffer_write_addr);
	ESP_LOGD(TAG, "<buffer_log");
}

uint32_t buffer_available(buffer_handle_t handle) {
	uint32_t available = handle->buffer_write_addr - handle->buffer_read_addr;
	return available;
}

uint32_t buffer_free(buffer_handle_t handle) {
	uint32_t free = handle->size - buffer_available(handle);
	return free;
}

void buffer_push(buffer_handle_t handle, uint8_t *data, uint32_t length) {
	assert(length <= buffer_free(handle));
	uint32_t current = handle->buffer_write_addr & handle->mask;
	spi_mem_write(handle->spi_mem_handle, current, length, data);
	handle->buffer_write_addr = (handle->buffer_write_addr + length);
}

void buffer_pull(buffer_handle_t handle, uint32_t length, uint8_t *data) {
	assert(length <= buffer_available(handle));
	uint32_t current = (handle->buffer_read_addr) & handle->mask;
	spi_mem_read(handle->spi_mem_handle, current, length, data);
	handle->buffer_read_addr = (handle->buffer_read_addr + length);
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
	buffer_handle->buffer_read_addr = 0;
	buffer_handle->buffer_write_addr = 0;

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
	handle->buffer_read_addr = 0;
	handle->buffer_write_addr = 0;
	free(handle);
	ESP_LOGD(TAG, "<buffer_end");
}

void buffer_reset(buffer_handle_t handle) {
	ESP_LOGD(TAG, ">buffer_reset");
	handle->buffer_read_addr = 0;
	handle->buffer_write_addr = 0;
	ESP_LOGD(TAG, "<buffer_reset");
}

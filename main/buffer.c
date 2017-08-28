// The author disclaims copyright to this source code.
#include "buffer.h"

static const char* TAG = "buffer.c";

/**
 * Buffer algorithm only works when memory size is a power of two.
 */
#define BUFFER_MASK (CONFIG_SPI_RAM_TOTAL_BYTES - 1)


uint32_t buffer_available(buffer_handle_t handle) {

	return handle->buffer_write_addr - handle->buffer_read_addr;
}

uint32_t buffer_free(buffer_handle_t handle) {
	return CONFIG_SPI_RAM_TOTAL_BYTES - (handle->buffer_write_addr - handle->buffer_read_addr);
}

void buffer_push(buffer_handle_t handle, uint8_t *data, uint32_t length) {
	assert(buffer_free(handle) >= length);
	spi_ram_data_write(handle->spi_device_handle, handle->buffer_write_addr, length, data);
	handle->buffer_write_addr = (handle->buffer_write_addr + length) & BUFFER_MASK;
}

void buffer_pull(buffer_handle_t handle, uint8_t *page, uint32_t length) {
	assert(buffer_available(handle) >= length);
	spi_ram_data_read(handle->spi_device_handle, handle->buffer_read_addr, length, page);
	handle->buffer_read_addr = (handle->buffer_read_addr + length) & BUFFER_MASK;
}

void buffer_log_configuration() {
	ESP_LOGD(TAG, ">buffer_log_configuration");
	//ESP_LOGI(TAG, "CONFIG_: %d", CONFIG_);
	ESP_LOGD(TAG, "<buffer_log_configuration");
}

void buffer_begin(spi_host_device_t host, buffer_handle_t *handle) {
	ESP_LOGD(TAG, ">buffer_begin");

	buffer_handle_t buffer_handle = malloc(sizeof(struct buffer_t));
	*handle = buffer_handle;
	buffer_handle->spi_host = host;
	buffer_handle->buffer_read_addr = 0;
	buffer_handle->buffer_write_addr = 0;

	spi_ram_begin_command(host, &(buffer_handle->spi_device_handle));
	spi_ram_command_write_mode_register(buffer_handle->spi_device_handle, SPI_RAM_MODE_SEQUENTIAL);
	spi_ram_command_read_mode_register(buffer_handle->spi_device_handle);
	spi_ram_end(buffer_handle->spi_device_handle);

	spi_ram_begin_data(host, &(buffer_handle->spi_device_handle));

	ESP_LOGD(TAG, "<buffer_begin");
}

void buffer_end(buffer_handle_t handle) {
	ESP_LOGD(TAG, ">buffer_end");
	spi_ram_end(handle->spi_device_handle);
	handle->spi_host = 0;
	handle->spi_device_handle = NULL;
	handle->buffer_read_addr = 0;
	handle->buffer_write_addr = 0;
	ESP_LOGD(TAG, "<buffer_end");
}

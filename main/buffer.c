// The author disclaims copyright to this source code.
#include "buffer.h"

static const char* TAG = "buffer.c";

/**
 * Buffer algorithm only works when memory size is a power of two.
 */
#define BUFFER_MASK (CONFIG_MEM_TOTAL_BYTES - 1)

uint32_t buffer_available(buffer_handle_t handle) {

	return handle->buffer_write_addr - handle->buffer_read_addr;
}

uint32_t buffer_free(buffer_handle_t handle) {
	return CONFIG_MEM_TOTAL_BYTES - (handle->buffer_write_addr - handle->buffer_read_addr);
}

void buffer_push(buffer_handle_t handle, uint8_t *data, uint32_t length) {
	assert(buffer_free(handle) >= length);
	spi_mem_write(handle->spi_mem_handle, handle->buffer_write_addr, length, data);
	handle->buffer_write_addr = (handle->buffer_write_addr + length) & BUFFER_MASK;
}

void buffer_pull(buffer_handle_t handle, uint8_t *page, uint32_t length) {
	assert(buffer_available(handle) >= length);
	spi_mem_read(handle->spi_mem_handle, handle->buffer_read_addr, length, page);
	handle->buffer_read_addr = (handle->buffer_read_addr + length) & BUFFER_MASK;
}

void buffer_log_configuration() {
	ESP_LOGD(TAG, ">buffer_log_configuration");
	//ESP_LOGI(TAG, "CONFIG_: %d", CONFIG_);
	ESP_LOGD(TAG, "<buffer_log_configuration");
}

void buffer_begin(spi_host_device_t host, buffer_handle_t *handle) {
	ESP_LOGD(TAG, ">buffer_begin");

	buffer_log_configuration();

	buffer_handle_t buffer_handle = malloc(sizeof(struct buffer_t));
	buffer_handle->spi_mem_handle = NULL;
	buffer_handle->buffer_read_addr = 0;
	buffer_handle->buffer_write_addr = 0;

	*handle = buffer_handle;

	spi_mem_config_t configuration;
	memset(&configuration, 0, sizeof(spi_mem_config_t));
	configuration.host = (spi_host_device_t) VSPI_HOST;
	configuration.clock_speed_hz = CONFIG_MEM_SPEED_MHZ * 1000000;
	configuration.spics_io_num = CONFIG_MEM_GPIO_CS;
	configuration.total_bytes = CONFIG_MEM_TOTAL_BYTES;
	configuration.number_of_pages = CONFIG_MEM_NUMBER_OF_PAGES;
	configuration.number_of_bytes_page = CONFIG_MEM_BYTES_PER_PAGE;
	spi_mem_begin(configuration, &(buffer_handle->spi_mem_handle));

	spi_mem_write_mode_register(buffer_handle->spi_mem_handle, SPI_MEM_MODE_SEQUENTIAL);
	spi_mem_read_mode_register(buffer_handle->spi_mem_handle);

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

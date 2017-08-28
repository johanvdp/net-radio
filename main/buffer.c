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
	spi_ram_write(handle->spi_ram_handle, handle->buffer_write_addr, length, data);
	handle->buffer_write_addr = (handle->buffer_write_addr + length) & BUFFER_MASK;
}

void buffer_pull(buffer_handle_t handle, uint8_t *page, uint32_t length) {
	assert(buffer_available(handle) >= length);
	spi_ram_read(handle->spi_ram_handle, handle->buffer_read_addr, length, page);
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
	buffer_handle->spi_ram_handle = NULL;
	buffer_handle->buffer_read_addr = 0;
	buffer_handle->buffer_write_addr = 0;

	*handle = buffer_handle;

	spi_ram_config_t configuration;
	memset(&configuration, 0, sizeof(spi_ram_config_t));
	configuration.host = (spi_host_device_t) VSPI_HOST;
	configuration.clock_speed_hz = CONFIG_SPI_RAM_SPEED_MHZ * 1000000;
	configuration.spics_io_num = CONFIG_SPI_RAM_GPIO_CS;
	configuration.total_bytes = CONFIG_SPI_RAM_TOTAL_BYTES;
	configuration.number_of_pages = CONFIG_SPI_RAM_NUMBER_OF_PAGES;
	configuration.number_of_bytes_page = CONFIG_SPI_RAM_BYTES_PER_PAGE;
	spi_ram_begin(configuration, &(buffer_handle->spi_ram_handle));

	spi_ram_write_mode_register(buffer_handle->spi_ram_handle, SPI_RAM_MODE_SEQUENTIAL);
	spi_ram_read_mode_register(buffer_handle->spi_ram_handle);

	ESP_LOGD(TAG, "<buffer_begin");
}

void buffer_end(buffer_handle_t handle) {
	ESP_LOGD(TAG, ">buffer_end");
	spi_ram_end(handle->spi_ram_handle);
	handle->spi_ram_handle = NULL;
	handle->buffer_read_addr = 0;
	handle->buffer_write_addr = 0;
	free(handle);
	ESP_LOGD(TAG, "<buffer_end");
}

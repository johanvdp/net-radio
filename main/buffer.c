// The author disclaims copyright to this source code.
#include "buffer.h"

static const char* TAG = "buffer.c";

/**
 * Buffer algorithm only works when memory size is a power of two.
 */
#define BUFFER_MASK (MEM_TOTAL_BYTES - 1)

uint32_t buffer_read_addr;
uint32_t buffer_write_addr;

uint32_t buffer_available() {
	return buffer_write_addr - buffer_read_addr;
}

uint32_t buffer_free() {
	return MEM_TOTAL_BYTES - (buffer_write_addr - buffer_read_addr);
}

void buffer_push(uint8_t *data, uint32_t length) {
	assert(buffer_free() >= length);
	mem_data_write(buffer_write_addr, length, data);
	buffer_write_addr = (buffer_write_addr + length) & BUFFER_MASK;
}

void buffer_pull(uint8_t *page, uint32_t length) {
	assert(buffer_available() >= length);
	mem_data_read(buffer_read_addr, length, page);
	buffer_read_addr = (buffer_read_addr + length) & BUFFER_MASK;
}

void buffer_log_configuration() {
	ESP_LOGD(TAG, ">buffer_log_configuration");
	//ESP_LOGI(TAG, "CONFIG_: %d", CONFIG_);
	ESP_LOGD(TAG, "<buffer_log_configuration");
}

void buffer_begin() {
	ESP_LOGD(TAG, ">buffer_begin");

	buffer_read_addr = 0;
	buffer_write_addr = 0;

	mem_begin_command((spi_host_device_t) VSPI_HOST);
	mem_command_write_mode_register(MEM_MODE_SEQUENTIAL);
	mem_command_read_mode_register();
	mem_end();

	mem_begin_data((spi_host_device_t) VSPI_HOST);

	ESP_LOGD(TAG, "<buffer_begin");
}

void buffer_end() {
	ESP_LOGD(TAG, ">buffer_end");
	mem_end();
	ESP_LOGD(TAG, "<buffer_end");
}

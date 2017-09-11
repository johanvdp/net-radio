// The author disclaims copyright to this source code.
#include "spi_mem.h"
#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"

static const char* TAG = "spi_mem";

/** Add device with same configuration but configured to transfer data only. */
static void spi_mem_add_command(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_add_command");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->spics_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_command)));
	ESP_LOGD(TAG, "<spi_mem_add_command");
}

/** Add device with same configuration but configured to transfer command and address in separate phase before data. */
static void spi_mem_add_data(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_add_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	// command and address in separate phase
	configuration.command_bits = 8;
	configuration.address_bits = 24;
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->spics_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_data)));
	ESP_LOGD(TAG, "<spi_mem_add_data");
}

static void spi_mem_remove_data(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_remove_data");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device_data));
	handle->device_data = NULL;
	ESP_LOGD(TAG, "<spi_mem_remove_data");
}

static void spi_mem_remove_command(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_remove_command");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device_command));
	handle->device_command = NULL;
	ESP_LOGD(TAG, "<spi_mem_remove_command");
}

void spi_mem_begin(spi_mem_config_t config, spi_mem_handle_t *handle) {
	ESP_LOGD(TAG, ">spi_mem_begin");
	ESP_LOGD(TAG, "host: %d", config.host);
	ESP_LOGD(TAG, "clock_speed_hz: %d", config.clock_speed_hz);
	ESP_LOGD(TAG, "spics_io_num: %d", config.spics_io_num);
	ESP_LOGD(TAG, "total_bytes: %d", config.total_bytes);
	ESP_LOGD(TAG, "number_of_pages: %d", config.number_of_pages);
	ESP_LOGD(TAG, "number_of_bytes_page: %d", config.number_of_bytes_page);

	// create a new handle
	spi_mem_t *spi_mem = malloc(sizeof(spi_mem_t));
	spi_mem->host = config.host;
	spi_mem->device_command = NULL;
	spi_mem->device_data = NULL;
	spi_mem->clock_speed_hz = config.clock_speed_hz;
	spi_mem->spics_io_num = config.spics_io_num;
	spi_mem->total_bytes = config.total_bytes;
	spi_mem->number_of_pages = config.number_of_pages;
	spi_mem->number_of_bytes_page = config.number_of_bytes_page;

	spi_mem_add_command(spi_mem);
	spi_mem_add_data(spi_mem);

	*handle = spi_mem;

	ESP_LOGD(TAG, "<spi_mem_begin");
}

void spi_mem_end(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_end");
	spi_mem_remove_command(handle);
	spi_mem_remove_data(handle);
	free(handle);
	ESP_LOGD(TAG, "<spi_mem_end");
}

uint8_t spi_mem_read_byte(spi_mem_handle_t handle, uint32_t address) {
	ESP_LOGV(TAG, ">spi_mem_read_byte");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.cmd = 0x03;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_RXDATA;
	transaction.length = 8;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
	ESP_LOGV(TAG, "<spi_mem_read_byte");
	return transaction.rx_data[0];
}

void spi_mem_read_page(spi_mem_handle_t handle, uint32_t address, uint8_t *data) {
	ESP_LOGV(TAG, ">spi_mem_read_page");
	spi_mem_read(handle, address, handle->number_of_bytes_page, data);
	ESP_LOGV(TAG, "<spi_mem_read_page");
}

void spi_mem_read(spi_mem_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	ESP_LOGV(TAG, ">spi_mem_read");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.cmd = 0x03;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = length * 8;
	transaction.rx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
	ESP_LOGV(TAG, "<spi_mem_read");
}

void spi_mem_write_byte(spi_mem_handle_t handle, uint32_t address, uint8_t data) {
	ESP_LOGV(TAG, ">spi_mem_write_byte");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.cmd = 0x02;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
	ESP_LOGV(TAG, "<spi_mem_write_byte");
}

void spi_mem_write_page(spi_mem_handle_t handle, uint32_t address, uint8_t *data) {
	ESP_LOGV(TAG, ">spi_mem_write_page");
	spi_mem_write(handle, address, handle->number_of_bytes_page, data);
	ESP_LOGV(TAG, "<spi_mem_write_page");
}

void spi_mem_write(spi_mem_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	ESP_LOGV(TAG, ">spi_mem_write");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.cmd = 0x02;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = 8 * length;
	transaction.tx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
	ESP_LOGV(TAG, "<spi_mem_write");
}

void spi_mem_enter_dual_io_access(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_enter_dual_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x3B;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_mem_enter_dual_io_access");
}

void spi_mem_enter_quad_io_access(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_enter_quad_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x38;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_mem_enter_quad_io_access");
}

void spi_mem_reset_io_access(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_reset_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0xFF;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_mem_reset_io_access");
}

spi_mem_mode_t spi_mem_read_mode_register(spi_mem_handle_t handle) {
	ESP_LOGD(TAG, ">spi_mem_read_mode_register");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x05;
	// uses rx_data, the contents will be in the second byte
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	uint8_t mode = transaction.rx_data[1];
	ESP_LOGD(TAG, "<spi_mem_read_mode_register 0x%02x", mode);
	return mode;
}

void spi_mem_write_mode_register(spi_mem_handle_t handle, spi_mem_mode_t mode) {
	ESP_LOGD(TAG, ">spi_mem_write_mode_register 0x%02x", mode);
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x01;
	transaction.tx_data[1] = mode;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_mem_write_mode_register");
}

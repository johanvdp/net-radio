// The author disclaims copyright to this source code.
#include <string.h>
#include "spi_ram.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_heap_caps.h"

static const char* TAG = "spi_ram.c";

/** Add device with same configuration but configured to transfer data only. */
void spi_ram_add_command(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_add_command");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->spics_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_command)));
	ESP_LOGD(TAG, "<spi_ram_add_command");
}

/** Add device with same configuration but configured to transfer command and address in separate phase before data. */
void spi_ram_add_data(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_add_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	// command and address in separate phase
	configuration.command_bits = 8;
	configuration.address_bits = 24;
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->spics_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device_data)));
	ESP_LOGD(TAG, "<spi_ram_add_data");
}

void spi_ram_remove_data(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_remove_data");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device_data));
	handle->device_data = NULL;
	ESP_LOGD(TAG, "<spi_ram_remove_data");
}

void spi_ram_remove_command(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_remove_command");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device_command));
	handle->device_command = NULL;
	ESP_LOGD(TAG, "<spi_ram_remove_command");
}

void spi_ram_begin(spi_ram_config_t config, spi_ram_handle_t *handle) {
	ESP_LOGD(TAG, ">spi_ram_begin");

	// create a new handle
	spi_ram_t *spi_ram = malloc(sizeof(spi_ram_t));
	spi_ram->host = config.host;
	spi_ram->device_command = NULL;
	spi_ram->device_data = NULL;
	spi_ram->clock_speed_hz = config.clock_speed_hz;
	spi_ram->spics_io_num = config.spics_io_num;
	spi_ram->total_bytes = config.total_bytes;
	spi_ram->number_of_pages = config.number_of_pages;
	spi_ram->number_of_bytes_page = config.number_of_bytes_page;

	spi_ram_add_command(spi_ram);
	spi_ram_add_data(spi_ram);

	*handle = spi_ram;

	ESP_LOGD(TAG, "<spi_ram_begin");
}

void spi_ram_end(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_end");
	spi_ram_remove_command(handle);
	spi_ram_remove_data(handle);
	free(handle);
	ESP_LOGD(TAG, "<spi_ram_end");
}

uint8_t spi_ram_read_byte(spi_ram_handle_t handle, uint32_t address) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x03;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_RXDATA;
	transaction.length = 8;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
	return transaction.rx_data[0];
}

void spi_ram_read_page(spi_ram_handle_t handle, uint32_t address, uint8_t *data) {
	spi_ram_read(handle, address, handle->number_of_bytes_page, data);
}

void spi_ram_read(spi_ram_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x03;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = length * 8;
	transaction.rx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
}

void spi_ram_write_byte(spi_ram_handle_t handle, uint32_t address, uint8_t data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x02;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
}

void spi_ram_write_page(spi_ram_handle_t handle, uint32_t address, uint8_t *data) {
	spi_ram_write(handle, address, handle->number_of_bytes_page, data);
}

void spi_ram_write(spi_ram_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x02;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = 8 * length;
	transaction.tx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_data, &transaction));
}

void spi_ram_enter_dual_io_access(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_enter_dual_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x3B;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_ram_enter_dual_io_access");
}

void spi_ram_enter_quad_io_access(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_enter_quad_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x38;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_ram_enter_quad_io_access");
}

void spi_ram_reset_io_access(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_reset_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0xFF;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_ram_reset_io_access");
}

spi_ram_mode_t spi_ram_read_mode_register(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_read_mode_register");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x05;
	// uses rx_data, the contents will be in the second byte
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	uint8_t mode = transaction.rx_data[1];
	ESP_LOGD(TAG, "<spi_ram_read_mode_register 0x%02x", mode);
	return mode;
}

void spi_ram_write_mode_register(spi_ram_handle_t handle, spi_ram_mode_t mode) {
	ESP_LOGD(TAG, ">spi_ram_write_mode_register 0x%02x", mode);
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x01;
	transaction.tx_data[1] = mode;
	// uses rx_data, the contents will be empty
	ESP_ERROR_CHECK(spi_device_transmit(handle->device_command, &transaction));
	ESP_LOGD(TAG, "<spi_ram_write_mode_register");
}

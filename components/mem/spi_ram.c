// The author disclaims copyright to this source code.
#include <string.h>
#include "spi_ram.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_heap_caps.h"

static const char* TAG = "spi_ram.c";

void spi_ram_begin(spi_ram_config_t config, spi_ram_handle_t *handle) {
	ESP_LOGD(TAG, ">spi_ram_begin");

	spi_ram_t *spi_ram = malloc(sizeof(spi_ram_t));
	spi_ram->host = config.host;
	spi_ram->device = NULL;
	spi_ram->clock_speed_hz = config.clock_speed_hz;
	spi_ram->spics_io_num = config.spics_io_num;
	spi_ram->total_bytes = config.total_bytes;
	spi_ram->number_of_pages = config.number_of_pages;
	spi_ram->number_of_bytes_page = config.number_of_bytes_page;

	*handle = spi_ram;

	ESP_LOGD(TAG, "<spi_ram_begin");
}

void spi_ram_end(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_end");
	handle->device = NULL;
	free(handle);
	ESP_LOGD(TAG, "<spi_ram_end");
}

void spi_ram_begin_command(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_begin_command");
	ESP_LOGD(TAG, "handle=%p", handle);
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->spics_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device)));
	ESP_LOGD(TAG, "<spi_ram_begin_command");
}

void spi_ram_begin_data(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_begin_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	// command and address in separate phase
	configuration.command_bits = 8;
	configuration.address_bits = 24;
	configuration.clock_speed_hz = handle->clock_speed_hz;
	configuration.spics_io_num = handle->spics_io_num;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(handle->host, &configuration, &(handle->device)));
	ESP_LOGD(TAG, "<spi_ram_begin_data");
}

void spi_ram_end_data(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_end_data");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device));
	handle->device = NULL;
	ESP_LOGD(TAG, "<spi_ram_end_data");
}

void spi_ram_end_command(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_end_command");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle->device));
	handle->device = NULL;
	ESP_LOGD(TAG, "<spi_ram_end_command");
}

uint8_t spi_ram_data_read_byte(spi_ram_handle_t handle, uint32_t address) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x03;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_RXDATA;
	transaction.length = 8;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
	return transaction.rx_data[0];
}

void spi_ram_data_read_page(spi_ram_handle_t handle, uint32_t address, uint8_t *data) {
	spi_ram_data_read(handle, address, CONFIG_SPI_RAM_BYTES_PER_PAGE, data);
}

void spi_ram_data_read(spi_ram_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x03;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = length * 8;
	transaction.rx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
}

void spi_ram_data_write_byte(spi_ram_handle_t handle, uint32_t address, uint8_t data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x02;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
}

void spi_ram_data_write_page(spi_ram_handle_t handle, uint32_t address, uint8_t *data) {
	spi_ram_data_write(handle, address, CONFIG_SPI_RAM_BYTES_PER_PAGE, data);
}

void spi_ram_data_write(spi_ram_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x02;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = 8 * length;
	transaction.tx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
}

void spi_ram_command_enter_dual_io_access(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_enter_dual_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x3B;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_enter_dual_io_access");
}

void spi_ram_command_enter_quad_io_access(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_enter_quad_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x38;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_enter_quad_io_access");
}

void spi_ram_command_reset_io_access(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_reset_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_reset_io_access");
}

uint8_t spi_ram_command_read_mode_register(spi_ram_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_read_mode_register");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
	uint8_t mode = transaction.rx_data[1];
	ESP_LOGD(TAG, "<spi_ram_command_read_mode_register 0x%02x", mode);
	return mode;
}

void spi_ram_command_write_mode_register(spi_ram_handle_t handle, uint8_t mode) {
	ESP_LOGD(TAG, ">spi_ram_command_write_mode_register 0x%02x", mode);
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x01;
	transaction.tx_data[1] = mode;
	ESP_ERROR_CHECK(spi_device_transmit(handle->device, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_write_mode_register");
}

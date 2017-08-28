// The author disclaims copyright to this source code.
#include <string.h>
#include "spi_ram.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_heap_caps.h"

static const char* TAG = "spi_ram.c";

/** SPI clock speed [Hz] */
#define SPI_RAM_SPEED_HZ (CONFIG_SPI_RAM_SPEED_MHZ * 1000000)

void spi_ram_log_configuration() {
	ESP_LOGD(TAG, ">spi_ram_log_configuration");
	ESP_LOGI(TAG, "CONFIG_SPI_RAM_GPIO_CS: %d", CONFIG_SPI_RAM_GPIO_CS);
	ESP_LOGI(TAG, "CONFIG_SPI_RAM_SPEED_MHZ: %d", CONFIG_SPI_RAM_SPEED_MHZ);
	ESP_LOGI(TAG, "CONFIG_SPI_RAM_TOTAL_BYTES: %d", CONFIG_SPI_RAM_TOTAL_BYTES);
	ESP_LOGI(TAG, "CONFIG_SPI_RAM_NUMBER_OF_PAGES: %d", CONFIG_SPI_RAM_NUMBER_OF_PAGES);
	ESP_LOGI(TAG, "CONFIG_SPI_RAM_BYTES_PER_PAGE: %d", CONFIG_SPI_RAM_BYTES_PER_PAGE);
	ESP_LOGD(TAG, "<spi_ram_log_configuration");
}

void spi_ram_begin_command(spi_host_device_t host, spi_device_handle_t *handle) {
	ESP_LOGD(TAG, ">spi_ram_begin_command %d", host);
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = SPI_RAM_SPEED_HZ;
	configuration.spics_io_num = CONFIG_SPI_RAM_GPIO_CS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, handle));
	ESP_LOGD(TAG, "<spi_ram_begin_command");
}

void spi_ram_begin_data(spi_host_device_t host, spi_device_handle_t *handle) {
	ESP_LOGD(TAG, ">spi_ram_begin_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	// command and address in separate phase
	configuration.command_bits = 8;
	configuration.address_bits = 24;
	configuration.clock_speed_hz = SPI_RAM_SPEED_HZ;
	configuration.spics_io_num = CONFIG_SPI_RAM_GPIO_CS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, handle));
	ESP_LOGD(TAG, "<spi_ram_begin_data");
}

void spi_ram_end(spi_device_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_end");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle));
	ESP_LOGD(TAG, "<spi_ram_end");
}

uint8_t spi_ram_data_read_byte(spi_device_handle_t handle, uint32_t address) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x03;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_RXDATA;
	transaction.length = 8;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	return transaction.rx_data[0];
}

void spi_ram_data_read_page(spi_device_handle_t handle, uint32_t address, uint8_t *data) {
	spi_ram_data_read(handle, address, CONFIG_SPI_RAM_BYTES_PER_PAGE, data);
}

void spi_ram_data_read(spi_device_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x03;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = length * 8;
	transaction.rx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
}

void spi_ram_data_write_byte(spi_device_handle_t handle, uint32_t address, uint8_t data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x02;
	transaction.addr = address;
	transaction.flags = SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
}

void spi_ram_data_write_page(spi_device_handle_t handle, uint32_t address, uint8_t *data) {
	spi_ram_data_write(handle, address, CONFIG_SPI_RAM_BYTES_PER_PAGE, data);
}

void spi_ram_data_write(spi_device_handle_t handle, uint32_t address, uint32_t length, uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.command = 0x02;
	transaction.addr = address;
	transaction.flags = 0;
	transaction.length = 8 * length;
	transaction.tx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
}

void spi_ram_command_enter_dual_io_access(spi_device_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_enter_dual_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x3B;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_enter_dual_io_access");
}

void spi_ram_command_enter_quad_io_access(spi_device_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_enter_quad_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0x38;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_enter_quad_io_access");
}

void spi_ram_command_reset_io_access(spi_device_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_reset_io_access");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 8;
	transaction.tx_data[0] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_reset_io_access");
}

uint8_t spi_ram_command_read_mode_register(spi_device_handle_t handle) {
	ESP_LOGD(TAG, ">spi_ram_command_read_mode_register");
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	uint8_t mode = transaction.rx_data[1];
	ESP_LOGD(TAG, "<spi_ram_command_read_mode_register 0x%02x", mode);
	return mode;
}

void spi_ram_command_write_mode_register(spi_device_handle_t handle, uint8_t mode) {
	ESP_LOGD(TAG, ">spi_ram_command_write_mode_register 0x%02x", mode);
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(transaction));
	transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	transaction.length = 16;
	transaction.tx_data[0] = 0x01;
	transaction.tx_data[1] = mode;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &transaction));
	ESP_LOGD(TAG, "<spi_ram_command_write_mode_register");
}

// The author disclaims copyright to this source code.
#include "mem.h"

static const char* TAG = "mem.c";

/** SPI clock speed [Hz] */
#define MEM_SPI_SPEED_HZ (CONFIG_MEM_SPI_SPEED_MHZ * 1000000)

spi_device_handle_t mem_spi_handle;
spi_transaction_t mem_spi_transaction;

void mem_log_configuration() {
	ESP_LOGD(TAG, ">mem_log_configuration");
	ESP_LOGI(TAG, "CONFIG_MEM_GPIO_CS: %d", CONFIG_MEM_GPIO_CS);
	ESP_LOGI(TAG, "CONFIG_MEM_SPI_SPEED_MHZ: %d", CONFIG_MEM_SPI_SPEED_MHZ);
	ESP_LOGD(TAG, "<mem_log_configuration");
}

void mem_begin_command(spi_host_device_t host) {
	ESP_LOGD(TAG, ">mem_begin_command %d", host);
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = MEM_SPI_SPEED_HZ;
	configuration.spics_io_num = CONFIG_MEM_GPIO_CS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	ESP_LOGD(TAG, "<mem_begin_command");
}

void mem_begin_data(spi_host_device_t host) {
	ESP_LOGD(TAG, ">mem_begin_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	// command and address in separate phase
	configuration.command_bits = 8;
	configuration.address_bits = 24;
	configuration.clock_speed_hz = MEM_SPI_SPEED_HZ;
	configuration.spics_io_num = CONFIG_MEM_GPIO_CS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	ESP_LOGD(TAG, "<mem_begin_data");
}

void mem_end() {
	ESP_LOGD(TAG, ">mem_end");
	ESP_ERROR_CHECK(spi_bus_remove_device(mem_spi_handle));
	ESP_LOGD(TAG, "<mem_end");
}

uint8_t mem_data_read_byte(uint32_t address) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x03;
	mem_spi_transaction.addr = address;
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA;
	mem_spi_transaction.length = 8;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	return mem_spi_transaction.rx_data[0];
}

void mem_data_read_page(uint32_t address, uint8_t *data) {
	mem_data_read(address, MEM_BYTES_PER_PAGE, data);
}

void mem_data_read(uint32_t address, uint32_t length, uint8_t *data) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x03;
	mem_spi_transaction.addr = address;
	mem_spi_transaction.flags = 0;
	mem_spi_transaction.length = length * 8;
	mem_spi_transaction.rx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

void mem_data_write_byte(uint32_t address, uint8_t data) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x02;
	mem_spi_transaction.addr = address;
	mem_spi_transaction.flags = SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = data;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

void mem_data_write_page(uint32_t address, uint8_t *data) {
	mem_data_write(address, MEM_BYTES_PER_PAGE, data);
}

void mem_data_write(uint32_t address, uint32_t length, uint8_t *data) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x02;
	mem_spi_transaction.addr = address;
	mem_spi_transaction.flags = 0;
	mem_spi_transaction.length = 8 * length;
	mem_spi_transaction.tx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

void mem_command_enter_dual_io_access() {
	ESP_LOGD(TAG, ">mem_command_enter_dual_io_access");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = 0x3B;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGD(TAG, "<mem_command_enter_dual_io_access");
}

void mem_command_enter_quad_io_access() {
	ESP_LOGD(TAG, ">mem_command_enter_quad_io_access");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = 0x38;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGD(TAG, "<mem_command_enter_quad_io_access");
}

void mem_command_reset_io_access() {
	ESP_LOGD(TAG, ">mem_command_reset_io_access");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGD(TAG, "<mem_command_reset_io_access");
}

uint8_t mem_command_read_mode_register() {
	ESP_LOGD(TAG, ">mem_command_read_mode_register");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	uint8_t mode = mem_spi_transaction.rx_data[1];
	ESP_LOGD(TAG, "<mem_command_read_mode_register 0x%02x", mode);
	return mode;
}

void mem_command_write_mode_register(uint8_t mode) {
	ESP_LOGD(TAG, ">mem_command_write_mode_register 0x%02x", mode);
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.tx_data[0] = 0x01;
	mem_spi_transaction.tx_data[1] = mode;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGD(TAG, "<mem_command_write_mode_register");
}

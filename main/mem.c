// The author disclaims copyright to this source code.
#include "mem.h"

static const char* TAG = "mem.c";

spi_device_handle_t mem_spi_handle;
spi_transaction_t mem_spi_transaction;

void mem_log_configuration() {
	ESP_LOGI(TAG, ">mem_log_configuration");
	ESP_LOGI(TAG, "CONFIG_GPIO_MEM_CS: %d", CONFIG_GPIO_MEM_CS);
	ESP_LOGI(TAG, "<mem_log_configuration");
}

void mem_initialize(spi_host_device_t host) {
	ESP_LOGI(TAG, ">mem_initialize");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	//configuration.address_bits = 0;
	//configuration.command_bits = 0;
	//configuration.dummy_bits = 0;
	configuration.mode = 0;
	//configuration.duty_cycle_pos = 0;
	//configuration.cs_ena_posttrans = 0;
	//configuration.cs_ena_pretrans = 0;
	configuration.clock_speed_hz = 1000000;
	configuration.spics_io_num = CONFIG_GPIO_MEM_CS;
	configuration.flags = 0;
	configuration.queue_size = 1;
	//configuration.pre_cb = NULL;
	//configuration.post_cb = NULL;
	memset(&mem_spi_handle, 0, sizeof(mem_spi_handle));
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	ESP_LOGI(TAG, "<mem_initialize");
}

void mem_free() {
	ESP_LOGI(TAG, ">mem_free");
	ESP_ERROR_CHECK(spi_bus_remove_device(mem_spi_handle));
	ESP_LOGI(TAG, "<mem_free");
}

// READ 0000 0011 0x03 Read data from memory array beginning at selected address
uint8_t mem_read_byte(uint32_t address) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	return mem_spi_transaction.rx_data[1];
}
// TODO: mem_read_page
// TODO: mem_read_sequence


// WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
// TODO: mem_write_byte
// TODO: mem_write_page
// TODO: mem_write_sequence

// EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
void mem_enter_dual_io_access() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x3B;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
void mem_enter_quad_io_access() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x38;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
void mem_reset_io_access() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// RDMR 0000 0101 0x05 Read Mode Register
uint8_t mem_read_mode_register() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	return mem_spi_transaction.rx_data[1];
}

// WRMR 0000 0001 0x01 Write Mode Register
void mem_write_mode_register(uint8_t mode) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x01;
	mem_spi_transaction.tx_data[1] = mode;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}


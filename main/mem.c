// The author disclaims copyright to this source code.
#include "mem.h"

static const char* TAG = "mem.c";

#define MEM_SPI_SPEED 20000000

spi_device_handle_t mem_spi_handle;
spi_transaction_t mem_spi_transaction;

void mem_log_configuration() {
	ESP_LOGI(TAG, ">mem_log_configuration");
	ESP_LOGI(TAG, "CONFIG_GPIO_MEM_CS: %d", CONFIG_GPIO_MEM_CS);
	ESP_LOGI(TAG, "<mem_log_configuration");
}

// initialize for subsequent data transactions
void mem_initialize_command(spi_host_device_t host) {
	ESP_LOGI(TAG, ">mem_initialize_command");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	//configuration.command_bits = 0;
	//configuration.address_bits = 0;
	//configuration.dummy_bits = 0;
	configuration.mode = 0;
	//configuration.duty_cycle_pos = 0;
	//configuration.cs_ena_posttrans = 0;
	//configuration.cs_ena_pretrans = 0;
	configuration.clock_speed_hz = MEM_SPI_SPEED;
	configuration.spics_io_num = CONFIG_GPIO_MEM_CS;
	configuration.flags = 0;
	configuration.queue_size = 1;
	//configuration.pre_cb = NULL;
	//configuration.post_cb = NULL;
	memset(&mem_spi_handle, 0, sizeof(mem_spi_handle));
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	ESP_LOGI(TAG, "<mem_initialize_command");
}

// initialize for subsequent command transactions
void mem_initialize_data(spi_host_device_t host) {
	ESP_LOGI(TAG, ">mem_initialize_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.command_bits = 8;
	configuration.address_bits = 24;
	//configuration.dummy_bits = 0;
	//configuration.mode = 0;
	//configuration.duty_cycle_pos = 0;
	//configuration.cs_ena_posttrans = 0;
	//configuration.cs_ena_pretrans = 0;
	configuration.clock_speed_hz = MEM_SPI_SPEED;
	configuration.spics_io_num = CONFIG_GPIO_MEM_CS;
	//configuration.flags = 0;
	configuration.queue_size = 1;
	//configuration.pre_cb = NULL;
	//configuration.post_cb = NULL;
	memset(&mem_spi_handle, 0, sizeof(mem_spi_handle));
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	ESP_LOGI(TAG, "<mem_initialize_data");
}

void mem_free() {
	ESP_LOGI(TAG, ">mem_free");
	ESP_ERROR_CHECK(spi_bus_remove_device(mem_spi_handle));
	ESP_LOGI(TAG, "<mem_free");
}

// READ 0000 0011 0x03 Read data from memory array beginning at selected address
uint8_t mem_data_read_byte(uint32_t address) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x03;
	mem_spi_transaction.address = address;
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA;
	mem_spi_transaction.length = 0;
	mem_spi_transaction.rxlength = 8;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	return mem_spi_transaction.rx_data[0];
}

void mem_data_read_page(uint32_t address, uint8_t *data) {
	mem_data_read(address, 32, data);
}

void mem_data_read(uint32_t address, uint32_t length, uint8_t *data) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x03;
	mem_spi_transaction.address = address;
	mem_spi_transaction.flags = 0;
	mem_spi_transaction.length =0 ;
	mem_spi_transaction.rxlength = length * 8;
	mem_spi_transaction.tx_buffer = 0;
	mem_spi_transaction.rx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
void mem_data_write_byte(uint32_t address, uint8_t data) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x02;
	mem_spi_transaction.address = address;
	mem_spi_transaction.flags = SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = data;
	mem_spi_transaction.rx_buffer = 0;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

void mem_data_write_page(uint32_t address, uint8_t *data) {
	mem_data_write(address, 32, data);
}

void mem_data_write(uint32_t address, uint32_t length, uint8_t *data) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x02;
	mem_spi_transaction.address = address;
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA;
	mem_spi_transaction.length = 8 * length;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_buffer = data;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
void mem_command_enter_dual_io_access() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x3B;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
void mem_command_enter_quad_io_access() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x38;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
void mem_command_reset_io_access() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// RDMR 0000 0101 0x05 Read Mode Register
uint8_t mem_command_read_mode_register() {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	return mem_spi_transaction.rx_data[1];
}

// WRMR 0000 0001 0x01 Write Mode Register
void mem_command_write_mode_register(uint8_t mode) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.rxlength = 0;
	mem_spi_transaction.tx_data[0] = 0x01;
	mem_spi_transaction.tx_data[1] = mode;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}


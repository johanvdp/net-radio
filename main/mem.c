// The author disclaims copyright to this source code.
#include "mem.h"

static const char* TAG = "mem.c";

#define MEM_SPI_SPEED (1000000)
#define MEM_MAX_DATA_LENGTH 2048
#define MEM_PREFIX_LENGTH 4

spi_device_handle_t mem_spi_handle;
spi_transaction_t mem_spi_transaction;
// empty buffer to transmit data while reading
uint8_t *workaroundBuffer;

void mem_workaround_malloc() {
	ESP_LOGI(TAG, ">mem_workaround_initialize");
	workaroundBuffer = mem_malloc(MEM_MAX_DATA_LENGTH);
	ESP_LOGI(TAG, "<mem_workaround_initialize");
}

void mem_workaround_free() {
	ESP_LOGI(TAG, ">mem_workaround_free");
	heap_caps_free(workaroundBuffer);
	ESP_LOGI(TAG, "<mem_workaround_free");
}

void *mem_malloc(size_t size) {
	ESP_LOGI(TAG, ">mem_malloc");

	size_t available = heap_caps_get_minimum_free_size(MALLOC_CAP_DMA);
	ESP_LOGI(TAG, "heap_caps_get_minimum_free_size: %d", available);
	uint32_t dataSize = size;
	uint32_t bufferSize = dataSize + MEM_PREFIX_LENGTH;
	ESP_LOGI(TAG, "data size: %d", dataSize);
	ESP_LOGI(TAG, "buffer size: %d", bufferSize);
	uint8_t *buffer = heap_caps_malloc(bufferSize, MALLOC_CAP_DMA);
	if (buffer == NULL) {
		ESP_LOGI(TAG, "heap_caps_malloc: out of memory");
	}
	memset(buffer, 0, bufferSize);
	ESP_LOGI(TAG, "buffer: %p", buffer);
	uint8_t *data = buffer + MEM_PREFIX_LENGTH;
	ESP_LOGI(TAG, "data: %p", data);

	ESP_LOGI(TAG, "<mem_malloc");
	return data;
}

void mem_free(void *ptr) {
	ESP_LOGI(TAG, ">mem_free");
	ESP_LOGI(TAG, "data: %p", ptr );
	ESP_LOGI(TAG, "buffer: %p", ptr - MEM_PREFIX_LENGTH);
	heap_caps_free(ptr - MEM_PREFIX_LENGTH);
	ESP_LOGI(TAG, "<mem_free");
}

void mem_log_configuration() {
	ESP_LOGI(TAG, ">mem_log_configuration");
	ESP_LOGI(TAG, "CONFIG_GPIO_MEM_CS: %d", CONFIG_GPIO_MEM_CS);
	ESP_LOGI(TAG, "<mem_log_configuration");
}

// initialize for subsequent data transactions
void mem_add_command(spi_host_device_t host) {
	ESP_LOGI(TAG, ">mem_initialize_command %d", host);
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.clock_speed_hz = MEM_SPI_SPEED;
	configuration.spics_io_num = CONFIG_GPIO_MEM_CS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	ESP_LOGI(TAG, "<mem_initialize_command");
}

// initialize for subsequent command transactions
void mem_add_data(spi_host_device_t host) {
	ESP_LOGI(TAG, ">mem_initialize_data");
	spi_device_interface_config_t configuration;
	memset(&configuration, 0, sizeof(configuration));
	configuration.command_bits = 0;
	configuration.address_bits = 0;
	configuration.clock_speed_hz = MEM_SPI_SPEED;
	configuration.spics_io_num = CONFIG_GPIO_MEM_CS;
	configuration.queue_size = 1;
	ESP_ERROR_CHECK(spi_bus_add_device(host, &configuration, &mem_spi_handle));
	mem_workaround_malloc();
	ESP_LOGI(TAG, "<mem_initialize_data");
}

void mem_remove() {
	ESP_LOGI(TAG, ">mem_remove");
	ESP_ERROR_CHECK(spi_bus_remove_device(mem_spi_handle));
	mem_workaround_free();
	ESP_LOGI(TAG, "<mem_remove");
}

// READ 0000 0011 0x03 Read data from memory array beginning at selected address
uint8_t mem_data_read_byte(uint32_t address) {
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.command = 0x03;
	mem_spi_transaction.addr = address;
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA;
	mem_spi_transaction.rxlength = 8;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	return mem_spi_transaction.rx_data[0];
}

void mem_data_read_page(uint32_t address, uint8_t *data) {
	mem_data_read(address, 32, data);
}

void mem_data_read(uint32_t address, uint32_t length, uint8_t *data) {
	ESP_LOGI(TAG, "data: %p", data);
	uint8_t *buffer = data - MEM_PREFIX_LENGTH;
	ESP_LOGI(TAG, "buffer: %p", buffer);
	workaroundBuffer[0] = 0x03;
	workaroundBuffer[1] = 0;
	workaroundBuffer[2] = 0;
	workaroundBuffer[3] = 0;

	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	//mem_spi_transaction.command = 0x03;
	//mem_spi_transaction.address = address;
	mem_spi_transaction.length = MEM_PREFIX_LENGTH + length;
	mem_spi_transaction.flags = 0;
	mem_spi_transaction.length = (MEM_PREFIX_LENGTH + length) * 8;
	mem_spi_transaction.tx_buffer = workaroundBuffer;
	mem_spi_transaction.rx_buffer = buffer;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
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
	mem_data_write(address, 32, data);
}

void mem_data_write(uint32_t address, uint32_t length, uint8_t *data) {
	ESP_LOGI(TAG, "data: %p", data);
	uint8_t *buffer = data - MEM_PREFIX_LENGTH;
	ESP_LOGI(TAG, "buffer: %p", buffer);
	buffer[0] = 0x02;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;

	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	//mem_spi_transaction.command = 0x02;
	//mem_spi_transaction.address = address;
	mem_spi_transaction.flags = 0;
	mem_spi_transaction.length = 8 * length;
	mem_spi_transaction.tx_buffer = buffer;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
}

// EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
void mem_command_enter_dual_io_access() {
	ESP_LOGI(TAG, ">mem_command_enter_dual_io_access");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = 0x3B;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGI(TAG, "<mem_command_enter_dual_io_access");
}

// EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
void mem_command_enter_quad_io_access() {
	ESP_LOGI(TAG, ">mem_command_enter_quad_io_access");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = 0x38;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGI(TAG, "<mem_command_enter_quad_io_access");
}

// RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
void mem_command_reset_io_access() {
	ESP_LOGI(TAG, ">mem_command_reset_io_access");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 8;
	mem_spi_transaction.tx_data[0] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGI(TAG, "<mem_command_reset_io_access");
}

// RDMR 0000 0101 0x05 Read Mode Register
uint8_t mem_command_read_mode_register() {
	ESP_LOGI(TAG, ">mem_command_read_mode_register");
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.tx_data[0] = 0x05;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	uint8_t mode = mem_spi_transaction.rx_data[1];
	ESP_LOGI(TAG, "<mem_command_read_mode_register 0x%02x", mode);
	return mode;
}

// WRMR 0000 0001 0x01 Write Mode Register
void mem_command_write_mode_register(uint8_t mode) {
	ESP_LOGI(TAG, ">mem_command_write_mode_register 0x%02x", mode);
	memset(&mem_spi_transaction, 0, sizeof(mem_spi_transaction));
	mem_spi_transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	mem_spi_transaction.length = 16;
	mem_spi_transaction.tx_data[0] = 0x01;
	mem_spi_transaction.tx_data[1] = mode;
	ESP_ERROR_CHECK(spi_device_transmit(mem_spi_handle, &mem_spi_transaction));
	ESP_LOGI(TAG, "<mem_command_write_mode_register");
}

// The author disclaims copyright to this source code.
#ifndef _MEM_H_
#define _MEM_H_

#include <string.h>
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"

#define MEM_TOTAL_BYTES 131072
#define MEM_NUMBER_OF_PAGES 4092
#define MEM_BYTES_PER_PAGE 32

#define MEM_MODE_BYTE (0x00)
#define MEM_MODE_PAGE (0x80)
#define MEM_MODE_SEQUENTIAL (0x40)

void mem_log_configuration();
void mem_initialize_command(spi_host_device_t host);
void mem_initialize_data(spi_host_device_t host);
void mem_free();

// READ 0000 0011 0x03 Read data from memory array beginning at selected address
uint8_t mem_data_read_byte(uint32_t address);
void mem_data_read_page(uint32_t address, uint8_t *data);
void mem_data_read(uint32_t address, uint32_t length, uint8_t *data);

// WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
void mem_data_write_byte(uint32_t address, uint8_t data);
void mem_data_write_page(uint32_t address, uint8_t *data);
void mem_data_write(uint32_t address, uint32_t length, uint8_t *data);

// EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
void mem_command_enter_dual_io_access();

// EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
void mem_command_enter_quad_io_access();

// RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
void mem_command_reset_io_access();

// RDMR 0000 0101 0x05 Read Mode Register
uint8_t mem_command_read_mode_register();

// WRMR 0000 0001 0x01 Write Mode Register
void mem_command_write_mode_register(uint8_t mode);

#endif

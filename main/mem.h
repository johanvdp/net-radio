// The author disclaims copyright to this source code.
#ifndef _MEM_H_
#define _MEM_H_

/**
 * @file
 * Driver to access 23LC1024 functions using the SPI bus.
 */

#include <string.h>
#include "esp_log.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_heap_caps.h"

#define MEM_TOTAL_BYTES (131072)
#define MEM_NUMBER_OF_PAGES (4092)
#define MEM_BYTES_PER_PAGE (32)

#define MEM_MODE_BYTE (0x00)
#define MEM_MODE_PAGE (0x80)
#define MEM_MODE_SEQUENTIAL (0x40)

/**
 * @brief Log configuration settings
 */
void mem_log_configuration();

/**
 * @brief Begin the _command_ transaction style
 * always _end every _begin
 * @param host Device handle
 */
void mem_begin_command(spi_host_device_t host);

/**
 * @brief begin the _data_ transaction style
 * always _end every _begin
 * @param host Device handle
 */
void mem_begin_data(spi_host_device_t host);

/**
 * @brief end transactions
 */
void mem_end();

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory one byte at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @return The value read
 */
uint8_t mem_data_read_byte(uint32_t address);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory one page at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param data Target for values read
 */
void mem_data_read_page(uint32_t address, uint8_t *data);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory sequentially
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param length The number of values read
 * @param data Target for values read
 */
void mem_data_read(uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory one byte at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param data The value written
 */
void mem_data_write_byte(uint32_t address, uint8_t data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory one page at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param data Source of values written
 */
void mem_data_write_page(uint32_t address, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory sequentially
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param length The number of values written
 * @param data Source of values written
 */
void mem_data_write(uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
 * assumes the _command_ transaction style is in effect
 */
void mem_command_enter_dual_io_access();

/**
 * @brief EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
 * assumes the _command_ transaction style is in effect
 */
void mem_command_enter_quad_io_access();

/**
 * @brief RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
 * assumes the _command_ transaction style is in effect
 */
void mem_command_reset_io_access();

/**
 * @brief RDMR 0000 0101 0x05 Read Mode Register
 * assumes the _command_ transaction style is in effect
 */
uint8_t mem_command_read_mode_register();

/**
 * @brief WRMR 0000 0001 0x01 Write Mode Register
 * assumes the _command_ transaction style is in effect
 * @param mode Access mode
 */
void mem_command_write_mode_register(uint8_t mode);

#endif

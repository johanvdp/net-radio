// The author disclaims copyright to this source code.
#ifndef _SPI_RAM_H_
#define _SPI_RAM_H_

/**
 * @file
 * Driver to access ram chip using the SPI bus.
 */

#include "driver/spi_common.h"

#define SPI_RAM_MODE_BYTE (0x00)
#define SPI_RAM_MODE_PAGE (0x80)
#define SPI_RAM_MODE_SEQUENTIAL (0x40)

/**
 * @brief Log configuration settings
 */
void spi_ram_log_configuration();

/**
 * @brief Begin the _command_ transaction style
 * always _end every _begin
 * @param host Device handle
 */
void spi_ram_begin_command(spi_host_device_t host);

/**
 * @brief begin the _data_ transaction style
 * always _end every _begin
 * @param host Device handle
 */
void spi_ram_begin_data(spi_host_device_t host);

/**
 * @brief end transactions
 */
void spi_ram_end();

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory one byte at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @return The value read
 */
uint8_t spi_ram_data_read_byte(uint32_t address);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory one page at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param data Target for values read
 */
void spi_ram_data_read_page(uint32_t address, uint8_t *data);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory sequentially
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param length The number of values read
 * @param data Target for values read
 */
void spi_ram_data_read(uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory one byte at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param data The value written
 */
void spi_ram_data_write_byte(uint32_t address, uint8_t data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory one page at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param data Source of values written
 */
void spi_ram_data_write_page(uint32_t address, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory sequentially
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param address Memory address
 * @param length The number of values written
 * @param data Source of values written
 */
void spi_ram_data_write(uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
 * assumes the _command_ transaction style is in effect
 */
void spi_ram_command_enter_dual_io_access();

/**
 * @brief EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
 * assumes the _command_ transaction style is in effect
 */
void spi_ram_command_enter_quad_io_access();

/**
 * @brief RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
 * assumes the _command_ transaction style is in effect
 */
void spi_ram_command_reset_io_access();

/**
 * @brief RDMR 0000 0101 0x05 Read Mode Register
 * assumes the _command_ transaction style is in effect
 */
uint8_t spi_ram_command_read_mode_register();

/**
 * @brief WRMR 0000 0001 0x01 Write Mode Register
 * assumes the _command_ transaction style is in effect
 * @param mode Access mode
 */
void spi_ram_command_write_mode_register(uint8_t mode);

#endif

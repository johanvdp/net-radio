// The author disclaims copyright to this source code.
#ifndef _SPI_RAM_H_
#define _SPI_RAM_H_

/**
 * @file
 * Driver to access ram chip using the SPI bus.
 */

#include "driver/spi_common.h"
#include "driver/spi_master.h"

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
 * @param host SPI host device handle
 * @param handle SPI device handle
 */
void spi_ram_begin_command(spi_host_device_t host, spi_device_handle_t *handle);

/**
 * @brief begin the _data_ transaction style
 * always _end every _begin
 * @param host SPI host device handle
 * @param handle SPI device handle
 */
void spi_ram_begin_data(spi_host_device_t host, spi_device_handle_t *handle);

/**
 * @brief end transactions
 * @param handle SPI device handle
 */
void spi_ram_end(spi_device_handle_t handle);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory one byte at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param handle SPI device handle
 * @param address Memory address
 * @return The value read
 */
uint8_t spi_ram_data_read_byte(spi_device_handle_t handle, uint32_t address);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory one page at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param handle SPI device handle
 * @param address Memory address
 * @param data Target for values read
 */
void spi_ram_data_read_page(spi_device_handle_t handle, uint32_t address, uint8_t *data);

/**
 * @brief READ 0000 0011 0x03 Read data from memory array beginning at selected address
 * read memory sequentially
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param handle SPI device handle
 * @param address Memory address
 * @param length The number of values read
 * @param data Target for values read
 */
void spi_ram_data_read(spi_device_handle_t handle, uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory one byte at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param handle SPI device handle
 * @param address Memory address
 * @param data The value written
 */
void spi_ram_data_write_byte(spi_device_handle_t handle, uint32_t address, uint8_t data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory one page at a time
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param handle SPI device handle
 * @param address Memory address
 * @param data Source of values written
 */
void spi_ram_data_write_page(spi_device_handle_t handle, uint32_t address, uint8_t *data);

/**
 * @brief WRITE 0000 0010 0x02 Write data to memory array beginning at selected address
 * write memory sequentially
 * assumes the memory device is already in the correct mode
 * assumes the _data_ transaction style is in effect
 * @param handle SPI device handle
 * @param address Memory address
 * @param length The number of values written
 * @param data Source of values written
 */
void spi_ram_data_write(spi_device_handle_t handle, uint32_t address, uint32_t length, uint8_t *data);

/**
 * @brief EDIO 0011 1011 0x3B Enter Dual I/O access (enter SDI bus mode)
 * assumes the _command_ transaction style is in effect
 * @param handle SPI device handle
 */
void spi_ram_command_enter_dual_io_access(spi_device_handle_t handle);

/**
 * @brief EQIO 0011 1000 0x38 Enter Quad I/O access (enter SQI bus mode)
 * assumes the _command_ transaction style is in effect
 * @param handle SPI device handle
 */
void spi_ram_command_enter_quad_io_access(spi_device_handle_t handle);

/**
 * @brief RSTIO 1111 1111 0xFF Reset Dual and Quad I/O access (revert to SPI bus mode)
 * assumes the _command_ transaction style is in effect
 * @param handle SPI device handle
 */
void spi_ram_command_reset_io_access(spi_device_handle_t handle);

/**
 * @brief RDMR 0000 0101 0x05 Read Mode Register
 * assumes the _command_ transaction style is in effect
 * @param handle SPI device handle
 */
uint8_t spi_ram_command_read_mode_register(spi_device_handle_t handle);

/**
 * @brief WRMR 0000 0001 0x01 Write Mode Register
 * assumes the _command_ transaction style is in effect
 * @param handle SPI device handle
 * @param mode Access mode
 */
void spi_ram_command_write_mode_register(spi_device_handle_t handle, uint8_t mode);

#endif
